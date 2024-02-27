#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""Description
Examples on how to use data_utils module
"""
import torch

from data.data_utils import LTRDataset, SPLIT_TYPE

def get_doc_num(dataset):
    ''' compute the number of documents in a dataset '''
    doc_num = 0
    for qid, torch_batch_rankings, torch_batch_std_labels, torch_batch_std_weights in dataset:
        doc_num += torch_batch_std_labels.size(1)

    return doc_num

def get_min_max_docs(train_dataset, vali_dataset, test_dataset, semi_supervised=False):
    ''' get the minimum / maximum number of documents per query '''
    min_doc = 10000000
    max_doc = 0
    sum_rele = 0
    if semi_supervised:
        sum_unknown = 0

    for qid, torch_batch_rankings, torch_batch_std_labels, torch_batch_std_weights in train_dataset:
        #print('torch_batch_std_labels', torch_batch_std_labels.size())
        doc_num = torch_batch_std_labels.size(1)
        min_doc = min(doc_num, min_doc)
        max_doc = max(max_doc, doc_num)
        sum_rele += (torch_batch_std_labels>0).sum()
        if semi_supervised:
            sum_unknown += (torch_batch_std_labels<0).sum()

    if vali_dataset is not None:
        for qid, torch_batch_rankings, torch_batch_std_labels, torch_batch_std_weights in vali_dataset:
            doc_num = torch_batch_std_labels.size(1)
            min_doc = min(doc_num, min_doc)
            max_doc = max(max_doc, doc_num)
            sum_rele += (torch_batch_std_labels>0).sum()
            if semi_supervised:
                sum_unknown += (torch_batch_std_labels < 0).sum()

    for qid, torch_batch_rankings, torch_batch_std_labels, torch_batch_std_weights in test_dataset:
        doc_num = torch_batch_std_labels.size(1)
        min_doc = min(doc_num, min_doc)
        max_doc = max(max_doc, doc_num)
        sum_rele += (torch_batch_std_labels>0).sum()
        if semi_supervised:
            sum_unknown += (torch_batch_std_labels<0).sum()

    if semi_supervised:
        return min_doc, max_doc, sum_rele.data.numpy(), sum_unknown.data.numpy()
    else:
        return min_doc, max_doc, sum_rele.data.numpy()

def get_label_distribution(train_dataset, vali_dataset, test_dataset, semi_supervised=False, max_lavel=None):
    ''' get the overall label distribution '''
    assert semi_supervised is False
    assert max_lavel is not None

    minlength = max_lavel + 1
    sum_bin_cnts = torch.zeros(minlength, dtype=torch.int)
    for qid, torch_batch_rankings, torch_batch_std_labels, torch_batch_std_weights in train_dataset:
        #print('torch_batch_std_labels', torch_batch_std_labels)
        bin_cnt = torch.bincount(torch.squeeze(torch_batch_std_labels, dim=0).type(torch.IntTensor), minlength=minlength)
        #print('qid', qid, 'bin-cnts:', bin_cnt)
        sum_bin_cnts = torch.add(sum_bin_cnts, bin_cnt)

    if vali_dataset is not None:
        for qid, torch_batch_rankings, torch_batch_std_labels, torch_batch_std_weights in vali_dataset:
            bin_cnt = torch.bincount(torch.squeeze(torch_batch_std_labels, dim=0).type(torch.IntTensor), minlength=minlength)
            sum_bin_cnts = torch.add(sum_bin_cnts, bin_cnt)

    for qid, torch_batch_rankings, torch_batch_std_labels, torch_batch_std_weights in test_dataset:
        bin_cnt = torch.bincount(torch.squeeze(torch_batch_std_labels, dim=0).type(torch.IntTensor), minlength=minlength)
        sum_bin_cnts = torch.add(sum_bin_cnts, bin_cnt)

    return sum_bin_cnts.data.numpy()

def get_min_max_feature(train_dataset, vali_dataset, test_dataset):
    ''' get the minimum / maximum feature values in a dataset '''
    min_f = 0
    max_f = 1000
    for qid, torch_batch_rankings, torch_batch_std_labels in train_dataset:
        mav = torch.max(torch_batch_rankings)
        if torch.isinf(mav):
            print(qid, mav)
        else:
            if mav > max_f: max_f = mav

        miv = torch.min(torch_batch_rankings)
        if miv < min_f: min_f = miv
    print('train', min_f, '\t', max_f)

    min_f = 0
    max_f = 1000
    for qid, torch_batch_rankings, torch_batch_std_labels in vali_dataset:
        mav = torch.max(torch_batch_rankings)
        if mav > max_f: max_f = mav
        miv = torch.min(torch_batch_rankings)
        if miv < min_f: min_f = miv
    print('vali', min_f, '\t', max_f)

    min_f = 0
    max_f = 1000
    for qid, torch_batch_rankings, torch_batch_std_labels in test_dataset:
        mav = torch.max(torch_batch_rankings)
        if mav > max_f: max_f = mav
        miv = torch.min(torch_batch_rankings)
        if miv < min_f: min_f = miv
    print('test', min_f, '\t', max_f)



def check_dataset_statistics(dir_data):
    '''
    Get the basic statistics on the specified dataset
    '''
    file_train, file_vali, file_test = dir_data + 'train.txt', dir_data + 'test.txt', dir_data + 'test.txt'

    train_dataset = LTRDataset(split_type=SPLIT_TYPE.Train, file=file_train)
    test_dataset =  LTRDataset(split_type=SPLIT_TYPE.Test, file=file_test)

    num_queries = train_dataset.__len__() + test_dataset.__len__()
    print('Total queries:\t', num_queries)
    print('\tTrain:', train_dataset.__len__(), 'Test:', test_dataset.__len__())

    num_docs = get_doc_num(train_dataset) + get_doc_num(test_dataset)
    print('Total docs:\t', num_docs)

    min_doc, max_doc, sum_rele = get_min_max_docs(train_dataset=train_dataset, vali_dataset=None, test_dataset=test_dataset)
    sum_bin_cnts = get_label_distribution(train_dataset=train_dataset, vali_dataset=None, test_dataset=test_dataset, semi_supervised=False, max_lavel=1)

    print('min, max documents per query', min_doc, max_doc)
    print('total relevant documents', sum_rele)
    print('avg rele documents per query', sum_rele * 1.0 / num_queries)
    print('avg documents per query', num_docs * 1.0 / num_queries)
    print('label distribution: ', sum_bin_cnts)

    for qid, torch_batch_rankings, torch_batch_std_labels, torch_batch_std_weights in train_dataset:
        print(qid)
        print(torch_batch_rankings)
        print(torch_batch_std_labels)
        print(torch_batch_std_weights)
        break


if __name__ == '__main__':
    dir_data = '/Users/chenglei/Downloads/'

    check_dataset_statistics(dir_data=dir_data)
