#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""Description
A general framework for evaluating traditional learning-to-rank methods.
"""

import os
import sys
import datetime
import numpy as np

import torch

#from tensorboardX import SummaryWriter
from base.ranker import LTRFRAME_TYPE
from metric.metric_utils import metric_results_to_string

from data.data_utils import LTRDataset, SPLIT_TYPE, LABEL_TYPE
from model.eval.eval_utils import ndcg_at_ks
from model.pairwise.ranknet import RankNet

class ContentRanker():
    def __init__(self, cuda=None, dir_data=None, batch_size=100, sf_para_dict=None, model_para_dict=None):
        if cuda is None:
            self.gpu, self.device = False, 'cpu'
        else:
            self.gpu, self.device = True, 'cuda:'+str(cuda)
            torch.cuda.set_device(cuda)

        self.sf_para_dict = sf_para_dict
        self.model_para_dict = model_para_dict
        self.dir_data = dir_data
        self.batch_size = batch_size

        self.ranker = RankNet(sf_para_dict=self.sf_para_dict, model_para_dict=self.model_para_dict, gpu=self.gpu, device=self.device)
        print("ranker model is ready")

        file_train, file_vali, file_test = dir_data + 'train2.txt', dir_data + 'test2.txt', dir_data + 'test2.txt'
        self.train_data = LTRDataset(file=file_train, split_type=SPLIT_TYPE.Train, batch_size=self.batch_size, shuffle=True)
        self.test_data = LTRDataset(file=file_test, split_type=SPLIT_TYPE.Test, batch_size=self.batch_size, shuffle=True)
        self.vali_data = None
        print("dataset is ready")

    def display_information(self, data_dict, model_para_dict):
        if self.gpu: print('-- GPU({}) is launched --'.format(self.device))
        print(' '.join(['\nStart {} on {} >>>'.format(model_para_dict['model_id'], data_dict['data_id'])]))

    def train(self, epochs=10, cutoffs=[4, 8, 16]):
        """
        A simple train and test, namely train based on training data & test based on testing data
        :param ranker:
        :param eval_dict:
        :param train_data:
        :param test_data:
        :param vali_data:
        :return:
        """
        self.ranker.reset_parameters()  # reset with the same random initialization

        assert self.train_data is not None
        assert self.test_data  is not None

        list_losses = []
        list_train_ndcgs = []
        list_test_ndcgs = []

        for i in range(epochs):
            epoch_loss = torch.zeros(1).to(self.device) if self.gpu else torch.zeros(1)
            for qid, batch_rankings, batch_stds, batch_weight in self.train_data:
                if self.gpu:
                    batch_rankings, batch_stds, batch_weight = batch_rankings.to(self.device), batch_stds.to(self.device), batch_weight.to(self.device)
                batch_loss, stop_training = self.ranker.train(batch_rankings, batch_stds, batch_weight)
                epoch_loss += batch_loss.item()

                #if stop_training:
                break
                #print(epoch_loss)

            np_epoch_loss = epoch_loss.cpu().numpy() if self.gpu else epoch_loss.data.numpy()
            list_losses.append(np_epoch_loss)

            test_ndcg_ks = ndcg_at_ks(ranker=self.ranker, test_data=self.test_data, ks=cutoffs, label_type=LABEL_TYPE.MultiLabel, gpu=self.gpu, device=self.device)
            np_test_ndcg_ks = test_ndcg_ks.data.numpy()
            list_test_ndcgs.append(np_test_ndcg_ks)

            train_ndcg_ks = ndcg_at_ks(ranker=self.ranker, test_data=self.train_data, ks=cutoffs, label_type=LABEL_TYPE.MultiLabel, gpu=self.gpu, device=self.device)
            np_train_ndcg_ks = train_ndcg_ks.data.numpy()
            list_train_ndcgs.append(np_train_ndcg_ks)

        test_ndcgs = np.vstack(list_test_ndcgs)
        train_ndcgs = np.vstack(list_train_ndcgs)

        print(list_losses)
        print(train_ndcgs)
        print(test_ndcgs)
        return list_losses, train_ndcgs, test_ndcgs


if __name__ == '__main__':
    dir_data = '/Users/chenglei/Downloads/'

    # num_features=None, h_dim=100, out_dim=1, num_layers=3, HD_AF='R', HN_AF='R', TL_AF='S', apply_tl_af=True, BN=True, RD=False, FBN=False
    ffnns_para = dict(num_features=113, num_layers=1, HD_AF='R', HN_AF='R', TL_AF='S', apply_tl_af=True, BN=False, RD=False, FBN=False)
    model_para = dict(sigma=1.0)
    model = ContentRanker(dir_data=dir_data, batch_size=100, sf_para_dict=ffnns_para, model_para_dict=model_para)

    model.train(epochs=3)
