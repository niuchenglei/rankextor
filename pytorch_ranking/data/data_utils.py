#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""Description

"""

import os
import random
import numpy as np
from pathlib import Path
from enum import Enum, unique, auto

from sklearn.preprocessing import MinMaxScaler, RobustScaler, StandardScaler

import torch
import torch.utils.data as data

from data.bin_utils import batch_count
from utils.numpy.np_extensions import np_arg_shuffle_ties
from data.one_hot_utils import get_one_hot_reprs
from utils.big_pickle import pickle_save, pickle_load

## Supported datasets and formats ##

GLTR_LIBSVM = ['LTR_LibSVM', 'LTR_LibSVM_K']
GLTR_LETOR  = ['LETOR', 'LETOR_K']

"""
GLTR refers to General Learning-to-rank, thus
GLTR_LIBSVM and GLTR_LETOR refer to general learning-to-rank datasets given in the formats of libsvm and LETOR, respectively.

The suffix '_K' indicates that the dataset consists of K folds in order to perform k-fold cross validation.

For GLTR_LIBSVM, it is defined as follows, where features with zero values are not included.
<ground-truth label int> qid:<query_id int> [<feature_id int>:<feature_value float>]

For example:

4 qid:105 2:0.4 8:0.7 50:0.5
1 qid:105 5:0.5 30:0.7 32:0.4 48:0.53
0 qid:210 4:0.9 38:0.01 39:0.5 45:0.7
1 qid:210 1:0.2 8:0.9 31:0.93 40:0.6

The above sample dataset includes two queries, the query “105” has 2 documents, the corresponding ground-truth labels are 4 and 1, respectively.

For GLTR_LETOR, it is defined as follows, where features with zero values are still included and the number of features per row must be the same.
<ground-truth label int> qid:<query_id int> [<feature_id int>:<feature_value float>]

4 qid:105 1:0.4 2:0.7  3:0.5
1 qid:105 1:0.5 2:0.7  3:0.4
0 qid:210 1:0.9 2:0.01 3:0.5
1 qid:210 1:0.2 2:0.9  3:0.93
"""

## supported feature normalization ##
SCALER_LEVEL = ['QUERY', 'DATASET']
SCALER_ID    = ['MinMaxScaler', 'RobustScaler', 'StandardScaler', "SLog1P"]

@unique
class MASK_TYPE(Enum):
    """ Supported ways of masking labels """
    rand_mask_all = auto()
    rand_mask_rele = auto()


@unique
class LABEL_TYPE(Enum):
    """ The types of labels of supported datasets """
    MultiLabel = auto()
    Permutation = auto()


@unique
class SPLIT_TYPE(Enum):
    """ The split-part of a dataset """
    Train = auto()
    Test = auto()
    Validation = auto()

class SymmetricLog1pScaler(object):
    """
    Symmetric Log1p Transformation
    {
    author = {Zhuang, Honglei and Wang, Xuanhui and Bendersky, Michael and Najork, Marc},
    title = {Feature Transformation for Neural Ranking Models},
    booktitle = {Proceedings of the 43rd SIGIR Conference},
    pages = {1649–1652}
    }
    """
    @staticmethod
    def fit_transform(X):
        return np.sign(X) * np.log(1.0 + np.abs(X))


def get_scaler(scaler_id):
    """ Initialize the scaler-object correspondingly """
    assert scaler_id in SCALER_ID
    if scaler_id == 'MinMaxScaler':
        scaler = MinMaxScaler()
    elif scaler_id == 'RobustScaler':
        scaler = RobustScaler()
    elif scaler_id == 'StandardScaler':
        scaler = StandardScaler()
    elif scaler_id == 'SLog1P':
        scaler = SymmetricLog1pScaler()

    return scaler

def get_scaler_setting(data_id, grid_search=False, scaler_id=None):
    """
    A default scaler-setting for loading a dataset
    :param data_id:
    :param grid_search: used for grid-search
    :return:
    """
    ''' According to {Introducing {LETOR} 4.0 Datasets}, "QueryLevelNorm version: Conduct query level normalization based on data in MIN version. This data can be directly used for learning. We further provide 5 fold partitions of this version for cross fold validation".
     --> Thus there is no need to perform query_level_scale again for {MQ2007_super | MQ2008_super | MQ2007_semi | MQ2008_semi}
     --> But for {MSLRWEB10K | MSLRWEB30K}, the query-level normalization is ## not conducted yet##.
     --> For {Yahoo_LTR_Set_1 | Yahoo_LTR_Set_1 }, the query-level normalization is already conducted.
     --> For Istella! LETOR, the query-level normalization is not conducted yet.
         We note that ISTELLA contains extremely large features, e.g., 1.79769313486e+308, we replace features of this kind with a constant 1000000.
    '''
    if grid_search:
        if scaler_id is None:
            choice_scale_data = [False]
            choice_scaler_id = [None]
            choice_scaler_level = [None]
        else:
            choice_scale_data = [True]  # True, False
            choice_scaler_id = [scaler_id]  # ['MinMaxScaler', 'RobustScaler', 'StandardScaler']
            choice_scaler_level = ['QUERY']  # SCALER_LEVEL = ['QUERY', 'DATASET']

        return choice_scale_data, choice_scaler_id, choice_scaler_level
    else:
        if scaler_id is None:
            scale_data = False
            scaler_id = None
            scaler_level = None
        else:
            scale_data = True
            scaler_level = 'QUERY'

        return scale_data, scaler_id, scaler_level

## ---------------------------------------------------- ##
""" processing on letor datasets """

def _parse_docid(comment):
    parts = comment.strip().split()
    return parts[2]

def _parse_qid_tok(tok):
    assert tok.startswith('qid:')
    return tok[4:]

def iter_lines(lines, has_targets=True, one_indexed=True, missing=0.0, has_comment=False):
    """
    Transforms an iterator of lines to an iterator of LETOR rows. Each row is represented by a (x, y, qid, comment) tuple.
    Parameters
    ----------
    lines : iterable of lines Lines to parse.
    has_targets : bool, optional, i.e., the relevance label
        Whether the file contains targets. If True, will expect the first token  every line to be a real representing
        the sample's target (i.e. score). If False, will use -1 as a placeholder for all targets.
    one_indexed : bool, optional, i.e., whether the index of the first feature is 1
        Whether feature ids are one-indexed. If True, will subtract 1 from each feature id.
    missing : float, optional
        Placeholder to use if a feature value is not provided for a sample.
    Yields
    ------
    x : array of floats Feature vector of the sample.
    y : float Target value (score) of the sample, or -1 if no target was parsed.
    qid : object Query id of the sample. This is currently guaranteed to be a string.
    comment : str Comment accompanying the sample.
    """
    for line in lines:
        #print(line)
        if has_comment:
            data, _, comment = line.rstrip().partition('#')
            toks = data.split()
        else:
            toks = line.rstrip().split()

        num_features = 0
        feature_vec = np.repeat(missing, 8)
        std_score = -1.0
        std_weight = 1.0
        if has_targets:
            if toks[0].find(':') != -1:
                _arr = toks[0].split(':')
                std_score = float(_arr[0])
                std_weight = float(_arr[1])
            else:
                std_score = float(toks[0])
            toks = toks[1:]

        qid = _parse_qid_tok(toks[0])

        for tok in toks[1:]:
            fid, _, val = tok.partition(':')
            fid = int(fid)
            val = float(val)
            if one_indexed:
                fid -= 1

            if fid < 0:
                print("error fid: {} {}".format(fid, tok))
            assert fid >= 0
            while len(feature_vec) <= fid:
                orig = len(feature_vec)
                feature_vec.resize(len(feature_vec) * 2)
                feature_vec[orig:orig * 2] = missing

            feature_vec[fid] = val
            num_features = max(fid + 1, num_features)

        assert num_features > 0
        feature_vec.resize(num_features)

        if has_comment:
            yield (feature_vec, std_score, qid, std_weight, comment)
        else:
            yield (feature_vec, std_score, qid, std_weight)

def parse_letor(source, has_targets=True, one_indexed=True, missing=0.0, has_comment=False):
    """
    Parses a LETOR dataset from `source`.
    Parameters
    ----------
    source : string or iterable of lines String, file, or other file-like object to parse.
    has_targets : bool, optional
    one_indexed : bool, optional
    missing : float, optional
    Returns
    -------
    X : array of arrays of floats Feature matrix (see `iter_lines`).
    y : array of floats Target vector (see `iter_lines`).
    qids : array of objects Query id vector (see `iter_lines`).
    comments : array of strs Comment vector (see `iter_lines`).
    """
    max_width = 0
    feature_vecs, std_scores, qids, std_weights = [], [], [], []
    if has_comment:
        comments = []

    it = iter_lines(source, has_targets=has_targets, one_indexed=one_indexed, missing=missing, has_comment=has_comment)
    for f_vec, s, qid, w in it:
        feature_vecs.append(f_vec)
        std_scores.append(s)
        std_weights.append(w)
        qids.append(qid)
        max_width = max(max_width, len(f_vec))

    assert max_width > 0
    all_features_mat = np.ndarray((len(feature_vecs), max_width), dtype=np.float64)
    all_features_mat.fill(missing)
    for i, x in enumerate(feature_vecs):
        all_features_mat[i, :len(x)] = x

    all_labels_vec = np.array(std_scores)
    all_weight_vec = np.array(std_weights)

    if has_comment:
        docids = [_parse_docid(comment) for comment in comments]
        #features, std_scores, qids, docids
        return all_features_mat, all_labels_vec, qids, all_weight_vec, docids
    else:
        # features, std_scores, qids
        return all_features_mat, all_labels_vec, qids, all_weight_vec

def iter_queries(in_file, has_targets=True, one_indexed=True, missing=0.0, has_comment=False):
    '''
    Transforms an iterator of rows to an iterator of queries (i.e., a unit of all the documents and labels associated
    with the same query). Each query is represented by a (qid, feature_mat, std_label_vec) tuple.
    :param in_file:
    :param has_comment:
    :param query_level_scale: perform query-level scaling, say normalization
    :param scaler: MinMaxScaler | RobustScaler
    :param unknown_as_zero: if not labled, regard the relevance degree as zero
    :return:
    '''

    list_Qs = []
    print(in_file)
    with open(in_file, encoding='iso-8859-1') as file_obj:
        dict_data = dict()

        all_features_mat, all_labels_vec, qids, all_weight_vec = parse_letor(file_obj.readlines(), has_targets, one_indexed, missing, has_comment)

        for i in range(len(qids)):
            f_vec = all_features_mat[i, :]
            std_s = all_labels_vec[i]
            std_w = all_weight_vec[i]
            qid = qids[i]

            if qid in dict_data:
                dict_data[qid].append((std_s, f_vec, std_w))
            else:
                dict_data[qid] = [(std_s, f_vec, std_w)]

        del all_features_mat
        # unique qids
        seen = set()
        seen_add = seen.add
        # sequential unique id
        qids_unique = [x for x in qids if not (x in seen or seen_add(x))]

        for qid in qids_unique:
            tmp = list(zip(*dict_data[qid]))
            list_labels_per_q = tmp[0]
            list_features_per_q = tmp[1]
            list_weight_per_q = tmp[2]
            feature_mat = np.vstack(list_features_per_q)

            Q = (qid, feature_mat, np.array(list_labels_per_q), np.array(list_weight_per_q))

            if Q is not None:
                list_Qs.append(Q)

    return list_Qs

## ---------------------------------------------------- ##

class LTRDataset(data.Dataset):
    """
    Loading the specified dataset as data.Dataset, a pytorch format.
    We assume that checking the meaningfulness of given loading-setting is conducted beforehand.
    """
    def __init__(self, split_type, file, batch_size=1, shuffle=False, has_targets=True, one_indexed=False, missing=0.0, has_comment=False):

        ''' split-specific settings '''
        self.split_type = split_type
        self.shuffle = shuffle
        self.batch_size = batch_size

        list_Qs = iter_queries(file, has_targets, one_indexed, missing, has_comment)

        self.list_torch_Qs = []
        list_inds = list(range(len(list_Qs)))
        for ind in list_inds:
            qid, doc_reprs, doc_labels, doc_weights = list_Qs[ind]

            if self.batch_size > 1:
                list_ranking = []
                list_labels = []
                list_weights = []
                for _ in range(self.batch_size):
                    des_inds = np_arg_shuffle_ties(doc_labels, descending=True)  # sampling by shuffling ties
                    list_ranking.append(doc_reprs[des_inds])
                    list_labels.append(doc_labels[des_inds])
                    list_weights.append(doc_weights[des_inds])

                batch_rankings = np.stack(list_ranking, axis=0)
                batch_std_labels = np.stack(list_labels, axis=0)
                batch_std_weights = np.stack(list_weights, axis=0)

                torch_batch_rankings = torch.from_numpy(batch_rankings).type(torch.FloatTensor)
                torch_batch_std_labels = torch.from_numpy(batch_std_labels).type(torch.FloatTensor)
                torch_batch_std_weights = torch.from_numpy(batch_std_weights).type(torch.FloatTensor)
            else:
                torch_batch_rankings = torch.from_numpy(doc_reprs).type(torch.FloatTensor)
                torch_batch_rankings = torch.unsqueeze(torch_batch_rankings, dim=0)  # a consistent batch dimension of size 1

                torch_batch_std_labels = torch.from_numpy(doc_labels).type(torch.FloatTensor)
                torch_batch_std_labels = torch.unsqueeze(torch_batch_std_labels, dim=0)

                torch_batch_std_weights = torch.from_numpy(doc_weights).type(torch.FloatTensor)
                torch_batch_std_weights = torch.unsqueeze(torch_batch_std_weights, dim=0)

            self.list_torch_Qs.append((qid, torch_batch_rankings, torch_batch_std_labels, torch_batch_std_weights))

        print('Num of q:', len(self.list_torch_Qs))

    def __len__(self):
        return len(self.list_torch_Qs)

    def __getitem__(self, index):
        qid, torch_batch_rankings, torch_batch_std_labels, torch_batch_std_weights = self.list_torch_Qs[index]
        return qid, torch_batch_rankings, torch_batch_std_labels, torch_batch_std_weights
