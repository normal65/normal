/**
 ***************************** Declaration ********************************
 * @file           : zh_code_table.h
 * @author         : FriedParrot (https://github.com/FriedParrot)
 * @license        : MIT license (https://opensource.org/license/mit)
 *****************************************************************************
 * @note  Ebook 移植说明：内容与上游 v1.0 一致（纯常量表，未修改）。
 *        来源：others/zh_pinyin_decoder/zh_pinyin_decoder/zh_code_table.h
 *****************************************************************************
 */
#ifndef __ZH_CODE_TABLE_H
#define __ZH_CODE_TABLE_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include <stdint.h>

    typedef struct
    {
        const uint8_t table_length; // length of code table
        const uint32_t char_start;  // start location of each character

        const char** code_table;       // following pinyin table
        const uint8_t* code_table_num; // number of chinese character
        const uint16_t* code_offset;   // each code table offset from start location
    } __code_index_t;

    extern const __code_index_t code_index[];

#ifdef __cplusplus
}
#endif //

#endif
