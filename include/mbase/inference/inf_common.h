#ifndef MBASE_INF_COMMON_H
#define MBASE_INF_COMMON_H

#include <mbase/common.h>
#include <mbase/vector.h>

// If llama.cpp is defined macro should be placed here
#include <llama.h>
#include <common/common.h>

MBASE_BEGIN

using inf_text_token = llama_token;
using inf_text_token_vector = mbase::vector<inf_text_token>;
using inf_embedding_vector = mbase::vector<F32>;

MBASE_END

#endif // MBASE_INF_COMMON_H