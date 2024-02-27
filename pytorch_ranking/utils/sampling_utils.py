#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""Description

"""

import torch

def arg_shuffle_ties(target_batch_stds, descending=True, gpu=False, device=None):
    ''' Shuffle ties, and return the corresponding indice '''
    batch_size, ranking_size = target_batch_stds.size()
    if batch_size > 1:
        list_rperms = []
        for _ in range(batch_size):
            list_rperms.append(torch.randperm(ranking_size))
        batch_rperms = torch.stack(list_rperms, dim=0)
    else:
        batch_rperms = torch.randperm(ranking_size).view(1, -1)

    if gpu: batch_rperms = batch_rperms.to(device)

    shuffled_target_batch_stds = torch.gather(target_batch_stds, dim=1, index=batch_rperms)
    batch_sorted_inds = torch.argsort(shuffled_target_batch_stds, descending=descending)
    batch_shuffle_ties_inds = torch.gather(batch_rperms, dim=1, index=batch_sorted_inds)

    return batch_shuffle_ties_inds

def unique_count(std_labels, descending=True):
    asc_std_labels, _ = torch.sort(std_labels)
    uni_elements, inds = torch.unique(asc_std_labels, sorted=True, return_inverse=True)
    asc_uni_cnts = torch.stack([(asc_std_labels == e).sum() for e in uni_elements])

    if descending:
        des_uni_cnts = torch.flip(asc_uni_cnts, dims=[0])
        return des_uni_cnts
    else:
        return asc_uni_cnts


def batch_global_unique_count(batch_std_labels, max_rele_lavel, descending=True, gpu=False):
    '''  '''
    batch_asc_std_labels, _ = torch.sort(batch_std_labels, dim=1)
    # default ascending order
    global_uni_elements = torch.arange(max_rele_lavel+1).type(torch.cuda.FloatTensor) if gpu else torch.arange(max_rele_lavel+1).type(torch.FloatTensor)

    asc_uni_cnts = torch.cat([(batch_asc_std_labels == e).sum(dim=1, keepdim=True) for e in global_uni_elements], dim=1) # row-wise count per element

    if descending:
        des_uni_cnts = torch.flip(asc_uni_cnts, dims=[1])
        return des_uni_cnts
    else:
        return asc_uni_cnts

def uniform_rand_per_label(uni_cnts):
    """ can be compatible with batch """
    num_unis = uni_cnts.size(0)  # number of unique elements
    inner_rand_inds = (torch.rand(num_unis) * uni_cnts.type(tensor)).type(
        torch.LongTensor)  # random index w.r.t each interval
    begs = torch.cumsum(torch.cat([tensor([0.]).type(torch.LongTensor), uni_cnts[0:num_unis - 1]]),
                        dim=0)  # begin positions of each interval within the same vector
    # print('begin positions', begs)
    rand_inds_per_label = begs + inner_rand_inds
    # print('random index', rand_inds_per_label)  # random index tensor([ 0,  1,  3,  6, 10]) ([0, 2, 3, 5, 8])

    return rand_inds_per_label


def sample_per_label(batch_rankings, batch_stds):
    assert 1 == batch_stds.size(0)

    std_labels = torch.squeeze(batch_stds)
    des_uni_cnts = unique_count(std_labels)
    rand_inds_per_label = uniform_rand_per_label(des_uni_cnts)

    sp_batch_rankings = batch_rankings[:, rand_inds_per_label, :]
    sp_batch_stds = batch_stds[:, rand_inds_per_label]

    return sp_batch_rankings, sp_batch_stds


if __name__ == '__main__':
    #1
    std_labels = tensor([3, 3, 2, 1, 1, 1, 0, 0, 0, 0])
    des_uni_cnts = unique_count(std_labels)
    print('des_uni_cnts', des_uni_cnts)

    batch_global_des_uni_cnts = batch_global_unique_count(std_labels.view(2, -1), max_rele_lavel=4)
    print(std_labels.view(2, -1))
    print('batch_global_des_uni_cnts', batch_global_des_uni_cnts)

    rand_inds_per_label = uniform_rand_per_label(des_uni_cnts)
    print('random elements', std_labels[rand_inds_per_label])
