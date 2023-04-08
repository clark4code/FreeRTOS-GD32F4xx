/**
 * @file fc_types.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2023-04-08
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once

typedef enum fc_error_t {
	FC_OK,
	FC_FAIL,
	FC_ERR_TIMEOUT,
	FC_ERR_INVALID_PARAM,
} fc_error_t;