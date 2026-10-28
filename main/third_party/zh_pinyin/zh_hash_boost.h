/**
 ***************************** Declaration ********************************
 * @file           : zh_hash_boost.h
 * @author         : FriedParrot (https://github.com/FriedParrot)
 * @license        : MIT license (https://opensource.org/license/mit)
 *****************************************************************************
 * @note  Ebook 移植说明：内容与上游 v1.2 一致（纯常量表，未修改）。
 *        来源：others/zh_pinyin_decoder/zh_pinyin_decoder/zh_hash_boost.h
 *****************************************************************************
 */
#ifndef __ZH_HASH_BOOST_H
#define __ZH_HASH_BOOST_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdlib.h>

#define ZH_HASH_TABLE_DIV 11

    typedef struct zh_hash_node_t
    {
        uint8_t num;
        const uint8_t* idx_list;
    } __zh_hash_node_t;

    extern const __zh_hash_node_t** hash_list[];

    uint8_t zh_hash_inlist(const char* str);

#ifdef __cplusplus
}
#endif

#endif
