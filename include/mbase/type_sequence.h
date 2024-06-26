#ifndef MBASE_TYPE_SEQUENCE_H
#define MBASE_TYPE_SEQUENCE_H

#include <mbase/common.h>
#include <mbase/sequence_iterator.h> // mbase::sequence_iterator, mbase::const_sequence_iterator, mbase::reverse_iterator, mbase::const_reverse_sequence_iterator
#include <mbase/allocator.h> // mbase::allocator

MBASE_STD_BEGIN

// TWO TYPES OF STRINGS
// BYTE SEQUENCE STRING (NULL TERMINATED)
// WIDE BYTE STRING

template<typename T, typename Alloc = allocator<T>>
class type_sequence {
public:
    using value_type = T;
    using allocator_type = Alloc;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using move_reference = T&&;
    using size_type = SIZE_T;
    using difference_type = PTRDIFF;
    using iterator = sequence_iterator<T>;
    using const_iterator = const_sequence_iterator<T>;
    using reverse_iterator = reverse_sequence_iterator<T>;
    using const_reverse_iterator = const_reverse_sequence_iterator<T>;

    static MBASE_INLINE_EXPR pointer copy_bytes(pointer out_target, const_pointer in_source, size_type in_length) noexcept 
    {
        return static_cast<pointer>(memcpy(out_target, in_source, sizeof(value_type) * in_length));
    }

    static MBASE_INLINE_EXPR pointer concat(pointer out_target, const_pointer in_source, size_type in_length) 
    {
        return static_cast<pointer>(strncat(out_target, in_source, sizeof(value_type) * in_length));
    }

    MBASE_ND(MBASE_RESULT_IGNORE) static MBASE_INLINE_EXPR bool is_equal(const_pointer in_lhs, const_pointer in_rhs, size_type in_length) noexcept 
    {
        if (!memcmp(in_lhs, in_rhs, sizeof(value_type) * in_length))
        {
            return true;
        }
        return false;
    }

    MBASE_ND(MBASE_RESULT_IGNORE) static MBASE_INLINE_EXPR bool is_equal(const_pointer in_lhs, const_pointer in_rhs) noexcept 
    {
        if (!strcmp(in_lhs, in_rhs))
        {
            return true;
        }
        return false;
    }

    MBASE_ND(MBASE_RESULT_IGNORE) static MBASE_INLINE_EXPR size_type length_bytes(const_pointer in_target) noexcept 
    {
        return strlen(in_target);
    }

    MBASE_ND(MBASE_RESULT_IGNORE) static MBASE_INLINE_EXPR I32 compare_bytes(const_pointer in_lhs, const_pointer in_rhs, size_type in_length) noexcept 
    {
        return memcmp(in_lhs, in_rhs, sizeof(value_type) * in_length);
    }

    MBASE_ND(MBASE_RESULT_IGNORE) static MBASE_INLINE_EXPR I32 compare_bytes(const_pointer in_lhs, const_pointer in_rhs) noexcept 
    {
        return strcmp(in_lhs, in_rhs);
    }

    static MBASE_INLINE_EXPR pointer fill(pointer in_target, value_type in_value, size_type in_length) noexcept 
    {
        memset(in_target, in_value, sizeof(value_type) * in_length);
        return in_target;
    }
};

template <>
class type_sequence<wchar_t> {

};

MBASE_STD_END

#endif // !MBASE_TYPE_SEQUENCE_H