#ifndef MBASE_LIST_ITERATOR_H
#define MBASE_LIST_ITERATOR_H

#include <mbase/common.h>

MBASE_STD_BEGIN

/*

    --- CLASS INFORMATION ---
Identification: S0C20-OBJ-NA-ST

Name: forward_list_iterator

Parent: None

Behaviour List:
- Default Constructible
- Copy Constructible
- Copy Assignable
- Arithmetic Operable
- Templated
- Type Aware
- Swappable
- Equality Comparable

Description:


*/


template<typename T, typename DataT>
class forward_list_iterator;

/*

    --- CLASS INFORMATION ---
Identification: S0C21-OBJ-NA-ST

Name: const_forward_list_iterator

Parent: None

Behaviour List:
- Default Constructible
- Copy Constructible
- Copy Assignable
- Arithmetic Operable
- Templated
- Type Aware
- Swappable
- Equality Comparable

Description:


*/


template<typename T, typename DataT>
class const_forward_list_iterator;

/*

    --- CLASS INFORMATION ---
Identification: S0C22-OBJ-NA-ST

Name: backward_list_iterator

Parent: None

Behaviour List:
- Default Constructible
- Copy Constructible
- Copy Assignable
- Arithmetic Operable
- Templated
- Type Aware
- Swappable
- Equality Comparable

Description:


*/

template<typename T, typename DataT>
class backward_list_iterator;

/*

    --- CLASS INFORMATION ---
Identification: S0C23-OBJ-NA-ST

Name: const_backward_list_iterator

Parent: None

Behaviour List:
- Default Constructible
- Copy Constructible
- Copy Assignable
- Arithmetic Operable
- Templated
- Type Aware
- Swappable
- Equality Comparable

Description:

*/

template<typename T, typename DataT>
class const_backward_list_iterator;

template<typename T, typename DataT>
class forward_list_iterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using size_type = SIZE_T;
    using difference_type = PTRDIFF;

    /* ===== BUILDER METHODS BEGIN ===== */
    MBASE_INLINE forward_list_iterator() noexcept;
    MBASE_INLINE forward_list_iterator(pointer in_ptr) noexcept;
    MBASE_INLINE forward_list_iterator(const forward_list_iterator& in_rhs) noexcept;
    /* ===== BUILDER METHODS END ===== */

    /* ===== OPERATOR BUILDER METHODS BEGIN ===== */
    MBASE_INLINE forward_list_iterator& operator=(const forward_list_iterator& in_rhs) noexcept;
    /* ===== OPERATOR BUILDER METHODS END ===== */

    /* ===== OBSERVATION METHODS BEGIN ===== */
    MBASE_INLINE pointer _get_node() const noexcept;
    MBASE_ND(MBASE_IGNORE_NONTRIVIAL) MBASE_INLINE DataT* get() noexcept;
    MBASE_ND(MBASE_IGNORE_NONTRIVIAL) MBASE_INLINE DataT& operator*() noexcept;
    MBASE_INLINE DataT* operator->() noexcept;
    /* ===== OBSERVATION METHODS END ===== */

    /* ===== OPERATOR STATE-MODIFIER METHODS BEGIN ===== */
    MBASE_ND(MBASE_RESULT_IGNORE) MBASE_INLINE forward_list_iterator operator+(difference_type in_rhs) noexcept;
    MBASE_INLINE forward_list_iterator& operator+=(difference_type in_rhs) noexcept;
    MBASE_INLINE forward_list_iterator& operator++() noexcept;
    MBASE_INLINE forward_list_iterator operator++(int) noexcept;
    /* ===== OPERATOR STATE-MODIFIER METHODS END ===== */

    /* ===== NON-MODIFIER METHODS BEGIN ===== */
    MBASE_ND(MBASE_RESULT_IGNORE) MBASE_INLINE bool operator==(const forward_list_iterator& in_rhs) const noexcept;
    MBASE_ND(MBASE_RESULT_IGNORE) MBASE_INLINE bool operator!=(const forward_list_iterator& in_rhs) const noexcept;
    MBASE_ND(MBASE_RESULT_IGNORE) MBASE_INLINE bool operator==(const const_forward_list_iterator<T, DataT>& in_rhs) const noexcept;
    MBASE_ND(MBASE_RESULT_IGNORE) MBASE_INLINE bool operator!=(const const_forward_list_iterator<T, DataT>& in_rhs) const noexcept;
    /* ===== NON-MODIFIER METHODS END ===== */

protected:
    pointer _ptr;
};

template<typename T, typename DataT>
class const_forward_list_iterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using size_type = SIZE_T;
    using difference_type = PTRDIFF;

    /* ===== BUILDER METHODS BEGIN ===== */
    const_forward_list_iterator() noexcept;
    const_forward_list_iterator(pointer in_ptr) noexcept;
    const_forward_list_iterator(const const_forward_list_iterator& in_rhs) noexcept;
    const_forward_list_iterator(const forward_list_iterator<T, DataT>& in_rhs) noexcept;
    /* ===== BUILDER METHODS END ===== */

    /* ===== OPERATOR BUILDER METHODS BEGIN ===== */
    MBASE_INLINE const_forward_list_iterator& operator=(const const_forward_list_iterator& in_rhs) noexcept;
    MBASE_INLINE const_forward_list_iterator& operator=(const forward_list_iterator<T, DataT>& in_rhs) noexcept;
    /* ===== OPERATOR BUILDER METHODS END ===== */

    /* ===== OBSERVATION METHODS BEGIN ===== */
    MBASE_INLINE const_pointer _get_node() const noexcept;
    MBASE_ND(MBASE_IGNORE_NONTRIVIAL) MBASE_INLINE const DataT* get() const noexcept;
    MBASE_ND(MBASE_IGNORE_NONTRIVIAL) MBASE_INLINE const DataT& operator*() const noexcept;
    MBASE_INLINE const DataT* operator->() const noexcept;
    /* ===== OBSERVATION METHODS END ===== */

    /* ===== STATE-MODIFIER METHODS BEGIN ===== */
    MBASE_ND(MBASE_RESULT_IGNORE) MBASE_INLINE const_forward_list_iterator operator+(difference_type in_rhs) noexcept;
    MBASE_INLINE const_forward_list_iterator& operator+=(difference_type in_rhs) noexcept;
    MBASE_INLINE const_forward_list_iterator& operator++() noexcept;
    MBASE_INLINE const_forward_list_iterator operator++(int) noexcept;
    /* ===== STATE-MODIFIER METHODS END ===== */

    /* ===== NON-MODIFIER METHODS BEGIN ===== */
    MBASE_ND(MBASE_RESULT_IGNORE) MBASE_INLINE bool operator==(const const_forward_list_iterator& in_rhs) const noexcept;
    MBASE_ND(MBASE_RESULT_IGNORE) MBASE_INLINE bool operator!=(const const_forward_list_iterator& in_rhs) const noexcept;
    /* ===== NON-MODIFIER METHODS END ===== */

protected:
    pointer _ptr;
};

template<typename T, typename DataT>
class backward_list_iterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using size_type = SIZE_T;
    using difference_type = PTRDIFF;

    /* ===== BUILDER METHODS BEGIN ===== */
    backward_list_iterator() noexcept;
    backward_list_iterator(pointer in_ptr) noexcept;
    backward_list_iterator(const backward_list_iterator& in_rhs) noexcept;
    /* ===== BUILDER METHODS END ===== */

    /* ===== OPERATOR BUILDER METHODS BEGIN ===== */
    MBASE_INLINE backward_list_iterator& operator=(const backward_list_iterator& in_rhs) noexcept;
    /* ===== OPERATOR BUILDER METHODS END ===== */

    /* ===== OBSERVATION METHODS BEGIN ===== */
    MBASE_INLINE pointer _get_node() const noexcept;
    MBASE_INLINE DataT* get() const noexcept;
    MBASE_ND(MBASE_IGNORE_NONTRIVIAL) MBASE_INLINE DataT& operator*() noexcept;
    MBASE_ND(MBASE_IGNORE_NONTRIVIAL) MBASE_INLINE DataT* operator->() noexcept;
    /* ===== OBSERVATION METHODS END ===== */

    /* ===== STATE-MODIFIER METHODS BEGIN ===== */
    MBASE_ND(MBASE_RESULT_IGNORE) MBASE_INLINE backward_list_iterator operator+(difference_type in_rhs) noexcept;
    MBASE_INLINE backward_list_iterator& operator+=(difference_type in_rhs) noexcept;
    MBASE_INLINE backward_list_iterator& operator++() noexcept;
    MBASE_INLINE backward_list_iterator operator++(int) noexcept;
    /* ===== STATE-MODIFIER METHODS END ===== */

    /* ===== NON-MODIFIER METHODS BEGIN ===== */
    MBASE_ND(MBASE_RESULT_IGNORE) MBASE_INLINE bool operator==(const backward_list_iterator& in_rhs) const noexcept;
    MBASE_ND(MBASE_RESULT_IGNORE) MBASE_INLINE bool operator!=(const backward_list_iterator& in_rhs) const noexcept;
    MBASE_ND(MBASE_RESULT_IGNORE) MBASE_INLINE bool operator==(const const_backward_list_iterator<T, DataT>& in_rhs) const noexcept;
    MBASE_ND(MBASE_RESULT_IGNORE) MBASE_INLINE bool operator!=(const const_backward_list_iterator<T, DataT>& in_rhs) const noexcept;
    /* ===== NON-MODIFIER METHODS END ===== */

protected:
    pointer _ptr;
};

template<typename T, typename DataT>
class const_backward_list_iterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using size_type = SIZE_T;
    using difference_type = PTRDIFF;

    /* ===== BUILDER METHODS BEGIN ===== */
    const_backward_list_iterator() noexcept;
    const_backward_list_iterator(pointer in_ptr) noexcept;
    const_backward_list_iterator(const const_backward_list_iterator& in_rhs) noexcept;
    const_backward_list_iterator(const backward_list_iterator<T, DataT>& in_rhs) noexcept;
    /* ===== BUILDER METHODS END ===== */

    /* ===== OPERATOR BUILDER METHODS BEGIN ===== */
    MBASE_INLINE const_backward_list_iterator& operator=(const const_backward_list_iterator& in_rhs) noexcept;
    MBASE_INLINE const_backward_list_iterator& operator=(const backward_list_iterator<T, DataT>& in_rhs) noexcept;
    /* ===== OPERATOR BUILDER METHODS END ===== */

    /* ===== OBSERVATION METHODS BEGIN ===== */
    MBASE_INLINE const_pointer _get_node() const noexcept;
    MBASE_ND(MBASE_IGNORE_NONTRIVIAL) MBASE_INLINE const DataT* get() const noexcept;
    MBASE_ND(MBASE_IGNORE_NONTRIVIAL) MBASE_INLINE const DataT& operator*() const noexcept;
    MBASE_INLINE const DataT* operator->() const noexcept;
    /* ===== OBSERVATION METHODS END ===== */

    /* ===== STATE-MODIFIER METHODS BEGIN ===== */
    MBASE_ND(MBASE_RESULT_IGNORE) MBASE_INLINE const_backward_list_iterator operator+(difference_type in_rhs) noexcept;
    MBASE_INLINE const_backward_list_iterator& operator+=(difference_type in_rhs) noexcept;
    MBASE_INLINE const_backward_list_iterator& operator++() noexcept;
    MBASE_INLINE const_backward_list_iterator operator++(int) noexcept;
    /* ===== STATE-MODIFIER METHODS END ===== */

    /* ===== NON-MODIFIER METHODS BEGIN ===== */
    MBASE_ND(MBASE_RESULT_IGNORE) MBASE_INLINE bool operator==(const const_backward_list_iterator& in_rhs) const noexcept;
    MBASE_ND(MBASE_RESULT_IGNORE) MBASE_INLINE bool operator!=(const const_backward_list_iterator& in_rhs) const noexcept;
    /* ===== NON-MODIFIER METHODS END ===== */

protected:
    pointer _ptr;
};

/*

    --- CLASS INFORMATION ---
Identification: S0C24-OBJ-NA-ST

Name: bi_list_iterator

Parent: S0C20-OBJ-NA-ST

Behaviour List:
- Default Constructible
- Copy Constructible
- Copy Assignable
- Arithmetic Operable
- Templated
- Type Aware
- Swappable
- Equality Comparable

Description:

*/


template<typename Type, typename DataT>
class bi_list_iterator : public forward_list_iterator<Type, DataT> {
public:
    using value_type = typename forward_list_iterator<Type, DataT>::value_type;
    using pointer = typename forward_list_iterator<Type, DataT>::pointer;
    using const_pointer = typename forward_list_iterator<Type, DataT>::const_pointer;
    using reference = typename forward_list_iterator<Type, DataT>::reference;
    using size_type = typename forward_list_iterator<Type, DataT>::size_type;
    using difference_type = typename forward_list_iterator<Type, DataT>::difference_type;
    using iterator_category = std::bidirectional_iterator_tag;

    /* ===== BUILDER METHODS BEGIN ===== */
    bi_list_iterator() noexcept;
    bi_list_iterator(pointer in_ptr) noexcept;
    bi_list_iterator(const bi_list_iterator& in_rhs) noexcept;
    /* ===== BUILDER METHODS END ===== */

    /* ===== OPERATOR BUILDER METHODS BEGIN ===== */
    MBASE_INLINE bi_list_iterator& operator=(const bi_list_iterator& in_rhs) noexcept;
    /* ===== OPERATOR BUILDER METHODS END ===== */

    /* ===== STATE-MODIFIER METHODS BEGIN ===== */
    MBASE_INLINE bi_list_iterator operator-(difference_type in_rhs) noexcept;
    MBASE_INLINE bi_list_iterator& operator-=(difference_type in_rhs) noexcept;
    MBASE_INLINE bi_list_iterator& operator--() noexcept;
    MBASE_INLINE bi_list_iterator operator--(int) noexcept;
    /* ===== STATE-MODIFIER METHODS END ===== */
};

/*

    --- CLASS INFORMATION ---
Identification: S0C25-OBJ-NA-ST

Name: const_bi_list_iterator

Parent: S0C21-OBJ-NA-ST

Behaviour List:
- Default Constructible
- Copy Constructible
- Copy Assignable
- Arithmetic Operable
- Templated
- Type Aware
- Swappable
- Equality Comparable

Description:

*/


template<typename Type, typename DataT>
class const_bi_list_iterator : public const_forward_list_iterator<Type, DataT> {
public:
    using value_type = typename const_forward_list_iterator<Type, DataT>::value_type;
    using pointer = typename const_forward_list_iterator<Type, DataT>::pointer;
    using const_pointer = typename const_forward_list_iterator<Type, DataT>::const_pointer;
    using reference = typename const_forward_list_iterator<Type, DataT>::reference;
    using size_type = typename const_forward_list_iterator<Type, DataT>::size_type;
    using difference_type = typename const_forward_list_iterator<Type, DataT>::difference_type;
    using iterator_category = std::bidirectional_iterator_tag;

    /* ===== BUILDER METHODS BEGIN ===== */
    const_bi_list_iterator() noexcept;
    const_bi_list_iterator(pointer in_ptr) noexcept;
    const_bi_list_iterator(const const_bi_list_iterator& in_rhs) noexcept;
    const_bi_list_iterator(const bi_list_iterator<Type, DataT>& in_rhs) noexcept;
    /* ===== BUILDER METHODS END ===== */

    /* ===== OPERATOR BUILDER METHODS BEGIN ===== */
    MBASE_INLINE const_bi_list_iterator& operator=(const const_bi_list_iterator& in_rhs) noexcept;
    MBASE_INLINE const_bi_list_iterator& operator=(const bi_list_iterator<Type, DataT>& in_rhs) noexcept;
    /* ===== OPERATOR BUILDER METHODS END ===== */

    /* ===== OPERATOR STATE-MODIFIER METHODS BEGIN ===== */
    MBASE_INLINE const_bi_list_iterator operator-(difference_type in_rhs) noexcept;
    MBASE_INLINE const_bi_list_iterator& operator-=(difference_type in_rhs) noexcept;
    MBASE_INLINE const_bi_list_iterator& operator--() noexcept;
    MBASE_INLINE const_bi_list_iterator operator--(int) noexcept;
    /* ===== OPERATOR STATE-MODIFIER METHODS END ===== */
};

/*

    --- CLASS INFORMATION ---
Identification: S0C26-OBJ-NA-ST

Name: reverse_bi_list_iterator

Parent: S0C22-OBJ-NA-ST

Behaviour List:
- Default Constructible
- Copy Constructible
- Copy Assignable
- Arithmetic Operable
- Templated
- Type Aware
- Swappable
- Equality Comparable

Description:

*/


template<typename Type, typename DataT>
class reverse_bi_list_iterator : public backward_list_iterator<Type, DataT> {
public:
    using value_type = typename backward_list_iterator<Type, DataT>::value_type;
    using pointer = typename backward_list_iterator<Type, DataT>::pointer;
    using const_pointer = typename backward_list_iterator<Type, DataT>::const_pointer;
    using reference = typename backward_list_iterator<Type, DataT>::reference;
    using size_type = typename backward_list_iterator<Type, DataT>::size_type;
    using difference_type = typename backward_list_iterator<Type, DataT>::difference_type;
    using iterator_category = std::bidirectional_iterator_tag;

    /* ===== BUILDER METHODS BEGIN ===== */
    reverse_bi_list_iterator() noexcept;
    reverse_bi_list_iterator(pointer in_ptr) noexcept;
    reverse_bi_list_iterator(const reverse_bi_list_iterator& in_rhs) noexcept;
    /* ===== BUILDER METHODS END ===== */

    /* ===== OPERATOR BUILDER METHODS BEGIN ===== */
    reverse_bi_list_iterator& operator=(const reverse_bi_list_iterator& in_rhs) noexcept;
    /* ===== OPERATOR BUILDER METHODS END ===== */

    /* ===== OPERATOR STATE-MODIFIER METHODS BEGIN ===== */
    MBASE_INLINE reverse_bi_list_iterator operator-(difference_type in_rhs) noexcept;
    MBASE_INLINE reverse_bi_list_iterator& operator-=(difference_type in_rhs) noexcept;
    MBASE_INLINE reverse_bi_list_iterator& operator--() noexcept;
    MBASE_INLINE reverse_bi_list_iterator operator--(int) noexcept;
    /* ===== OPERATOR STATE-MODIFIER METHODS END ===== */
};

/*

    --- CLASS INFORMATION ---
Identification: S0C27-OBJ-NA-ST

Name: const_reverse_bi_list_iterator

Parent: S0C23-OBJ-NA-ST

Behaviour List:
- Default Constructible
- Copy Constructible
- Copy Assignable
- Arithmetic Operable
- Templated
- Type Aware
- Swappable
- Equality Comparable

Description:

*/


template<typename Type, typename DataT>
class const_reverse_bi_list_iterator : public const_backward_list_iterator<Type, DataT> {
public:
    using value_type = typename const_backward_list_iterator<Type, DataT>::value_type;
    using pointer = typename const_backward_list_iterator<Type, DataT>::pointer;
    using const_pointer = typename const_backward_list_iterator<Type, DataT>::const_pointer;
    using reference = typename const_backward_list_iterator<Type, DataT>::reference;
    using size_type = typename const_backward_list_iterator<Type, DataT>::size_type;
    using difference_type = typename const_backward_list_iterator<Type, DataT>::difference_type;
    using iterator_category = std::bidirectional_iterator_tag;

    /* ===== BUILDER METHODS BEGIN ===== */
    const_reverse_bi_list_iterator() noexcept;
    const_reverse_bi_list_iterator(pointer in_ptr) noexcept;
    const_reverse_bi_list_iterator(const const_reverse_bi_list_iterator& in_rhs) noexcept;
    const_reverse_bi_list_iterator(const reverse_bi_list_iterator<Type, DataT>& in_rhs) noexcept;
    /* ===== BUILDER METHODS END ===== */

    /* ===== OPERATOR BUILDER METHODS BEGIN ===== */
    const_reverse_bi_list_iterator& operator=(const const_reverse_bi_list_iterator& in_rhs) noexcept;
    const_reverse_bi_list_iterator& operator=(const reverse_bi_list_iterator<Type, DataT>& in_rhs) noexcept;
    /* ===== OPERATOR BUILDER METHODS END ===== */

    MBASE_INLINE const_pointer get() const noexcept;

    /* ===== STATE-MODIFIER METHODS BEGIN ===== */
    MBASE_INLINE const_reverse_bi_list_iterator operator-(difference_type in_rhs) noexcept;
    MBASE_INLINE const_reverse_bi_list_iterator& operator-=(difference_type in_rhs) noexcept;
    MBASE_INLINE const_reverse_bi_list_iterator& operator--() noexcept;
    MBASE_INLINE const_reverse_bi_list_iterator operator--(int) noexcept;
    /* ===== STATE-MODIFIER METHODS END ===== */
};

/* <-- FORWARD LIST ITERATOR IMPLEMENTATION --> */
template<typename T, typename DataT>
MBASE_INLINE forward_list_iterator<T, DataT>::forward_list_iterator() noexcept : _ptr(nullptr)
{
}

template<typename T, typename DataT>
MBASE_INLINE forward_list_iterator<T, DataT>::forward_list_iterator(pointer in_ptr) noexcept : _ptr(in_ptr) 
{
}

template<typename T, typename DataT>
MBASE_INLINE forward_list_iterator<T, DataT>::forward_list_iterator(const forward_list_iterator& in_rhs) noexcept : _ptr(in_rhs._ptr)
{
}

template<typename T, typename DataT>
MBASE_INLINE forward_list_iterator<T, DataT>& forward_list_iterator<T, DataT>::operator=(const forward_list_iterator& in_rhs) noexcept
{
    _ptr = in_rhs._ptr;
    return *this;
}

template<typename T, typename DataT>
MBASE_INLINE typename forward_list_iterator<T, DataT>::pointer forward_list_iterator<T, DataT>::_get_node() const noexcept 
{
    return _ptr;
}

template<typename T, typename DataT>
MBASE_ND(MBASE_IGNORE_NONTRIVIAL) MBASE_INLINE DataT* forward_list_iterator<T, DataT>::get() noexcept 
{
    return &_ptr->data;
}

template<typename T, typename DataT>
MBASE_ND(MBASE_IGNORE_NONTRIVIAL) MBASE_INLINE DataT& forward_list_iterator<T, DataT>::operator*() noexcept 
{
    return _ptr->data;
}

template<typename T, typename DataT>
MBASE_INLINE DataT* forward_list_iterator<T, DataT>::operator->() noexcept 
{
    return this->get();
}

template<typename T, typename DataT>
MBASE_ND(MBASE_RESULT_IGNORE) MBASE_INLINE forward_list_iterator<T, DataT> forward_list_iterator<T, DataT>::operator+(difference_type in_rhs) noexcept 
{

    forward_list_iterator fli(*this);
    fli += in_rhs;
    return fli;
}

template<typename T, typename DataT>
MBASE_INLINE forward_list_iterator<T, DataT>& forward_list_iterator<T, DataT>::operator+=(difference_type in_rhs) noexcept 
{
    for (size_type i = 0; i < in_rhs; i++)
    {
        _ptr = _ptr->next;
    }
    return *this;
}

template<typename T, typename DataT>
MBASE_INLINE forward_list_iterator<T, DataT>& forward_list_iterator<T, DataT>::operator++() noexcept 
{
    _ptr = _ptr->next;
    return *this;
}

template<typename T, typename DataT>
MBASE_INLINE forward_list_iterator<T, DataT> forward_list_iterator<T, DataT>::operator++(int) noexcept 
{
    forward_list_iterator fli(*this);
    ++(*this);
    return fli;
}

template<typename T, typename DataT>
MBASE_ND(MBASE_RESULT_IGNORE) MBASE_INLINE bool forward_list_iterator<T, DataT>::operator==(const forward_list_iterator& in_rhs) const noexcept 
{
    return _ptr == in_rhs._get_node();
}

template<typename T, typename DataT>
MBASE_ND(MBASE_RESULT_IGNORE) MBASE_INLINE bool forward_list_iterator<T, DataT>::operator!=(const forward_list_iterator& in_rhs) const noexcept 
{
    return _ptr != in_rhs._get_node();
}

template<typename T, typename DataT>
MBASE_ND(MBASE_RESULT_IGNORE) MBASE_INLINE bool forward_list_iterator<T, DataT>::operator==(const const_forward_list_iterator<T, DataT>& in_rhs) const noexcept 
{
    return _ptr == in_rhs._get_node();
}

template<typename T, typename DataT>
MBASE_ND(MBASE_RESULT_IGNORE) MBASE_INLINE bool forward_list_iterator<T, DataT>::operator!=(const const_forward_list_iterator<T, DataT>& in_rhs) const noexcept 
{
    return _ptr != in_rhs._get_node();
}

/* <-- CONST FORWARD LIST ITERATOR IMPLEMENTATION --> */
template<typename T, typename DataT>
const_forward_list_iterator<T, DataT>::const_forward_list_iterator() noexcept : _ptr(nullptr)
{
}

template<typename T, typename DataT>
const_forward_list_iterator<T, DataT>::const_forward_list_iterator(pointer in_ptr) noexcept : _ptr(in_ptr) 
{
}

template<typename T, typename DataT>
const_forward_list_iterator<T, DataT>::const_forward_list_iterator(const const_forward_list_iterator& in_rhs) noexcept : _ptr(in_rhs._ptr) 
{
}

template<typename T, typename DataT>
const_forward_list_iterator<T, DataT>::const_forward_list_iterator(const forward_list_iterator<T, DataT>& in_rhs) noexcept : _ptr(in_rhs.get()) 
{
}

template<typename T, typename DataT>
MBASE_INLINE const_forward_list_iterator<T, DataT>& const_forward_list_iterator<T, DataT>::operator=(const const_forward_list_iterator& in_rhs) noexcept
{
    _ptr = in_rhs._ptr;
    return *this;
}

template<typename T, typename DataT>
MBASE_INLINE const_forward_list_iterator<T, DataT>& const_forward_list_iterator<T, DataT>::operator=(const forward_list_iterator<T, DataT>& in_rhs) noexcept
{
    _ptr = in_rhs._ptr;
    return *this;
}

template<typename T, typename DataT>
MBASE_ND(MBASE_IGNORE_NONTRIVIAL) MBASE_INLINE typename const_forward_list_iterator<T, DataT>::const_pointer const_forward_list_iterator<T, DataT>::_get_node() const noexcept 
{
    return _ptr;
}

template<typename T, typename DataT>
MBASE_ND(MBASE_IGNORE_NONTRIVIAL) MBASE_INLINE const DataT* const_forward_list_iterator<T, DataT>::get() const noexcept 
{
    return &_ptr->data;
}

template<typename T, typename DataT>
MBASE_ND(MBASE_IGNORE_NONTRIVIAL) MBASE_INLINE const DataT& const_forward_list_iterator<T, DataT>::operator*() const noexcept 
{
    return _ptr->data;
}

template<typename T, typename DataT>
MBASE_INLINE const DataT* const_forward_list_iterator<T, DataT>::operator->() const noexcept 
{
    return this->get();
}

template<typename T, typename DataT>
MBASE_ND(MBASE_RESULT_IGNORE) MBASE_INLINE const_forward_list_iterator<T, DataT> const_forward_list_iterator<T, DataT>::operator+(difference_type in_rhs) noexcept 
{
    const_forward_list_iterator cfli(*this);
    cfli += in_rhs;
    return cfli;
}

template<typename T, typename DataT>
MBASE_INLINE const_forward_list_iterator<T, DataT>& const_forward_list_iterator<T, DataT>::operator+=(difference_type in_rhs) noexcept
{
    for (size_type i = 0; i < in_rhs; i++)
    {
        _ptr = _ptr->next;
    }
    return *this;
}

template<typename T, typename DataT>
MBASE_INLINE const_forward_list_iterator<T, DataT>& const_forward_list_iterator<T, DataT>::operator++() noexcept
{
    _ptr = _ptr->next;
    return *this;
}

template<typename T, typename DataT>
MBASE_INLINE const_forward_list_iterator<T, DataT> const_forward_list_iterator<T, DataT>::operator++(int) noexcept 
{
    const_forward_list_iterator fli(*this);
    ++(*this);
    return fli;
}

template<typename T, typename DataT>
MBASE_ND(MBASE_RESULT_IGNORE) MBASE_INLINE bool const_forward_list_iterator<T, DataT>::operator==(const const_forward_list_iterator& in_rhs) const noexcept 
{
    return _ptr == in_rhs._get_node();
}

template<typename T, typename DataT>
MBASE_ND(MBASE_RESULT_IGNORE) MBASE_INLINE bool const_forward_list_iterator<T, DataT>::operator!=(const const_forward_list_iterator& in_rhs) const noexcept
{
    return _ptr != in_rhs._get_node();
}

/* <-- REVERSE LIST ITERATOR IMPLEMENTATION --> */
template<typename T, typename DataT>
backward_list_iterator<T, DataT>::backward_list_iterator() noexcept : _ptr(nullptr)
{
}

template<typename T, typename DataT>
backward_list_iterator<T, DataT>::backward_list_iterator(pointer in_ptr) noexcept : _ptr(in_ptr) 
{
}

template<typename T, typename DataT>
backward_list_iterator<T, DataT>::backward_list_iterator(const backward_list_iterator& in_rhs) noexcept : _ptr(in_rhs._ptr) 
{
}

template<typename T, typename DataT>
MBASE_INLINE backward_list_iterator<T, DataT>& backward_list_iterator<T, DataT>::operator=(const backward_list_iterator& in_rhs) noexcept
{
    _ptr = in_rhs._ptr;
    return *this;
}

template<typename T, typename DataT>
MBASE_INLINE typename backward_list_iterator<T, DataT>::pointer backward_list_iterator<T, DataT>::_get_node() const noexcept 
{
    return _ptr;
}

template<typename T, typename DataT>
MBASE_INLINE DataT* backward_list_iterator<T, DataT>::get() const noexcept 
{
    return &_ptr->data;
}

template<typename T, typename DataT>
MBASE_ND(MBASE_IGNORE_NONTRIVIAL) MBASE_INLINE DataT& backward_list_iterator<T, DataT>::operator*() noexcept 
{
    return _ptr->data;
}

template<typename T, typename DataT>
MBASE_ND(MBASE_IGNORE_NONTRIVIAL) MBASE_INLINE DataT* backward_list_iterator<T, DataT>::operator->() noexcept
{
    return this->get();
}

template<typename T, typename DataT>
MBASE_ND(MBASE_RESULT_IGNORE) MBASE_INLINE backward_list_iterator<T, DataT> backward_list_iterator<T, DataT>::operator+(difference_type in_rhs) noexcept 
{
    backward_list_iterator bli(*this);
    bli += in_rhs;
    return bli;
}

template<typename T, typename DataT>
MBASE_INLINE backward_list_iterator<T, DataT>& backward_list_iterator<T, DataT>::operator+=(difference_type in_rhs) noexcept 
{
    for (size_type i = 0; i < in_rhs; i++)
    {
        _ptr = _ptr->prev;
    }
    return *this;
}

template<typename T, typename DataT>
MBASE_INLINE backward_list_iterator<T, DataT>& backward_list_iterator<T, DataT>::operator++() noexcept 
{
    _ptr = _ptr->prev;
    return *this;
}

template<typename T, typename DataT>
MBASE_INLINE backward_list_iterator<T, DataT> backward_list_iterator<T, DataT>::operator++(int) noexcept 
{
    backward_list_iterator fli(*this);
    ++(*this);
    return fli;
}

template<typename T, typename DataT>
MBASE_ND(MBASE_RESULT_IGNORE) MBASE_INLINE bool backward_list_iterator<T, DataT>::operator==(const backward_list_iterator& in_rhs) const noexcept 
{
    return _ptr == in_rhs._ptr;
}

template<typename T, typename DataT>
MBASE_ND(MBASE_RESULT_IGNORE) MBASE_INLINE bool backward_list_iterator<T, DataT>::operator!=(const backward_list_iterator& in_rhs) const noexcept 
{
    return _ptr != in_rhs._ptr;
}

template<typename T, typename DataT>
MBASE_ND(MBASE_RESULT_IGNORE) MBASE_INLINE bool backward_list_iterator<T, DataT>::operator==(const const_backward_list_iterator<T, DataT>& in_rhs) const noexcept 
{
    return _ptr == in_rhs._get_node();
}

template<typename T, typename DataT>
MBASE_ND(MBASE_RESULT_IGNORE) MBASE_INLINE bool backward_list_iterator<T, DataT>::operator!=(const const_backward_list_iterator<T, DataT>& in_rhs) const noexcept 
{
    return _ptr != in_rhs._get_node();
}

/* <-- CONST REVERSE LIST ITERATOR IMPLEMENTATION --> */
template<typename T, typename DataT>
const_backward_list_iterator<T, DataT>::const_backward_list_iterator() noexcept : _ptr(nullptr)
{
}

template<typename T, typename DataT>
const_backward_list_iterator<T, DataT>::const_backward_list_iterator(pointer in_ptr) noexcept : _ptr(in_ptr) 
{
}

template<typename T, typename DataT>
const_backward_list_iterator<T, DataT>::const_backward_list_iterator(const const_backward_list_iterator& in_rhs) noexcept : _ptr(in_rhs._ptr)
{
}

template<typename T, typename DataT>
const_backward_list_iterator<T, DataT>::const_backward_list_iterator(const backward_list_iterator<T, DataT>& in_rhs) noexcept : _ptr(in_rhs.get()) 
{
}

template<typename T, typename DataT>
MBASE_INLINE const_backward_list_iterator<T, DataT>& const_backward_list_iterator<T, DataT>::operator=(const const_backward_list_iterator& in_rhs) noexcept
{
    _ptr = in_rhs._ptr;
    return *this;
}

template<typename T, typename DataT>
MBASE_INLINE const_backward_list_iterator<T, DataT>& const_backward_list_iterator<T, DataT>::operator=(const backward_list_iterator<T, DataT>& in_rhs) noexcept
{
    _ptr = in_rhs._ptr;
    return *this;
}

template<typename T, typename DataT>
MBASE_ND(MBASE_IGNORE_NONTRIVIAL) MBASE_INLINE typename const_backward_list_iterator<T, DataT>::const_pointer const_backward_list_iterator<T, DataT>::_get_node() const noexcept 
{
    return _ptr;
}

template<typename T, typename DataT>
MBASE_ND(MBASE_IGNORE_NONTRIVIAL) MBASE_INLINE const DataT* const_backward_list_iterator<T, DataT>::get() const noexcept
{
    return &_ptr->data;
}

template<typename T, typename DataT>
MBASE_ND(MBASE_IGNORE_NONTRIVIAL) MBASE_INLINE const DataT& const_backward_list_iterator<T, DataT>::operator*() const noexcept 
{
    return _ptr->data;
}

template<typename T, typename DataT>
MBASE_INLINE const DataT* const_backward_list_iterator<T, DataT>::operator->() const noexcept
{
    return this->get();
}

template<typename T, typename DataT>
MBASE_ND(MBASE_RESULT_IGNORE) MBASE_INLINE const_backward_list_iterator<T, DataT> const_backward_list_iterator<T, DataT>::operator+(difference_type in_rhs) noexcept
{
    const_backward_list_iterator cbli(*this);
    cbli += in_rhs;
    return cbli;
}

template<typename T, typename DataT>
MBASE_INLINE const_backward_list_iterator<T, DataT>& const_backward_list_iterator<T, DataT>::operator+=(difference_type in_rhs) noexcept
{
    for (size_type i = 0; i < in_rhs; i++)
    {
        _ptr = _ptr->prev;
    }
    return *this;
}

template<typename T, typename DataT>
MBASE_INLINE const_backward_list_iterator<T, DataT>& const_backward_list_iterator<T, DataT>::operator++() noexcept
{
    _ptr = _ptr->prev;
    return *this;
}

template<typename T, typename DataT>
MBASE_INLINE const_backward_list_iterator<T, DataT> const_backward_list_iterator<T, DataT>::operator++(int) noexcept
{
    const_backward_list_iterator cbli(*this);
    ++(*this);
    return cbli;
}

template<typename T, typename DataT>
MBASE_ND(MBASE_RESULT_IGNORE) MBASE_INLINE bool const_backward_list_iterator<T, DataT>::operator==(const const_backward_list_iterator& in_rhs) const noexcept
{
    return _ptr == in_rhs._get_node();
}

template<typename T, typename DataT>
MBASE_ND(MBASE_RESULT_IGNORE) MBASE_INLINE bool const_backward_list_iterator<T, DataT>::operator!=(const const_backward_list_iterator& in_rhs) const noexcept
{
    return _ptr != in_rhs._get_node();
}

/* <-- BIDIRECTIONAL LIST ITERATOR IMPLEMENTATION -->*/
template<typename Type, typename DataT>
bi_list_iterator<Type, DataT>::bi_list_iterator() noexcept : forward_list_iterator<Type, DataT>()
{
}

template<typename Type, typename DataT>
bi_list_iterator<Type, DataT>::bi_list_iterator(pointer in_ptr) noexcept : forward_list_iterator<Type, DataT>(in_ptr) 
{
}

template<typename Type, typename DataT>
bi_list_iterator<Type, DataT>::bi_list_iterator(const bi_list_iterator& in_rhs) noexcept : forward_list_iterator<Type, DataT>(in_rhs._ptr)
{
}

template<typename Type, typename DataT>
MBASE_INLINE bi_list_iterator<Type, DataT>& bi_list_iterator<Type, DataT>::operator=(const bi_list_iterator& in_rhs) noexcept
{
    this->_ptr = in_rhs._ptr;
    return *this;
}

template<typename Type, typename DataT>
MBASE_INLINE bi_list_iterator<Type, DataT> bi_list_iterator<Type, DataT>::operator-(difference_type in_rhs) noexcept
{
    bi_list_iterator bli(*this);
    bli -= in_rhs;
    return bli;
}

template<typename Type, typename DataT>
MBASE_INLINE bi_list_iterator<Type, DataT>& bi_list_iterator<Type, DataT>::operator-=(difference_type in_rhs) noexcept
{
    for (size_type i = 0; i < in_rhs; i++)
    {
        this->_ptr = this->_ptr->prev;
    }
    return *this;
}

template<typename Type, typename DataT>
MBASE_INLINE bi_list_iterator<Type, DataT>& bi_list_iterator<Type, DataT>::operator--() noexcept 
{
    this->_ptr = this->_ptr->prev;
    return *this;
}

template<typename Type, typename DataT>
MBASE_INLINE bi_list_iterator<Type, DataT> bi_list_iterator<Type, DataT>::operator--(int) noexcept 
{
    bi_list_iterator bi(*this);
    --(*this);
    return bi;
}

/* <-- CONST BIDIRECTIONAL LIST ITERATOR IMPLEMENTATION -->*/
template<typename Type, typename DataT>
const_bi_list_iterator<Type, DataT>::const_bi_list_iterator() noexcept : const_forward_list_iterator<Type, DataT>()
{
}

template<typename Type, typename DataT>
const_bi_list_iterator<Type, DataT>::const_bi_list_iterator(pointer in_ptr) noexcept : const_forward_list_iterator<Type, DataT>(in_ptr) 
{
}

template<typename Type, typename DataT>
const_bi_list_iterator<Type, DataT>::const_bi_list_iterator(const const_bi_list_iterator& in_rhs) noexcept : const_forward_list_iterator<Type, DataT>(in_rhs._ptr) 
{
}

template<typename Type, typename DataT>
const_bi_list_iterator<Type, DataT>::const_bi_list_iterator(const bi_list_iterator<Type, DataT>& in_rhs) noexcept : const_forward_list_iterator<Type, DataT>(in_rhs.get()) 
{
}

template<typename Type, typename DataT>
MBASE_INLINE const_bi_list_iterator<Type, DataT>& const_bi_list_iterator<Type, DataT>::operator=(const const_bi_list_iterator& in_rhs) noexcept
{
    this->_ptr = in_rhs._ptr;
    return *this;
}

template<typename Type, typename DataT>
MBASE_INLINE const_bi_list_iterator<Type, DataT>& const_bi_list_iterator<Type, DataT>::operator=(const bi_list_iterator<Type, DataT>& in_rhs) noexcept
{
    this->_ptr = in_rhs._ptr;
    return *this;
}

template<typename Type, typename DataT>
MBASE_INLINE const_bi_list_iterator<Type, DataT> const_bi_list_iterator<Type, DataT>::operator-(difference_type in_rhs) noexcept 
{
    const_bi_list_iterator bli(*this);
    bli -= in_rhs;
    return bli;
}

template<typename Type, typename DataT>
MBASE_INLINE const_bi_list_iterator<Type, DataT>& const_bi_list_iterator<Type, DataT>::operator-=(difference_type in_rhs) noexcept
{
    for (size_type i = 0; i < in_rhs; i++)
    {
        this->_ptr = this->_ptr->prev;
    }
    return *this;
}

template<typename Type, typename DataT>
MBASE_INLINE const_bi_list_iterator<Type, DataT>& const_bi_list_iterator<Type, DataT>::operator--() noexcept
{
    this->_ptr = this->_ptr->prev;
    return *this;
}

template<typename Type, typename DataT>
MBASE_INLINE const_bi_list_iterator<Type, DataT> const_bi_list_iterator<Type, DataT>::operator--(int) noexcept
{
    const_bi_list_iterator bli(*this);
    --(*this);
    return bli;
}

/* <-- REVERSE BIDIRECTIONAL LIST ITERATOR IMPLEMENTATION -->*/
template<typename Type, typename DataT>
reverse_bi_list_iterator<Type, DataT>::reverse_bi_list_iterator() noexcept : backward_list_iterator<Type, DataT>()
{
}

template<typename Type, typename DataT>
reverse_bi_list_iterator<Type, DataT>::reverse_bi_list_iterator(pointer in_ptr) noexcept : backward_list_iterator<Type, DataT>(in_ptr) 
{
}

template<typename Type, typename DataT>
reverse_bi_list_iterator<Type, DataT>::reverse_bi_list_iterator(const reverse_bi_list_iterator& in_rhs) noexcept : backward_list_iterator<Type, DataT>(in_rhs._ptr) 
{
}

template<typename Type, typename DataT>
reverse_bi_list_iterator<Type, DataT>& reverse_bi_list_iterator<Type, DataT>::operator=(const reverse_bi_list_iterator& in_rhs) noexcept
{
    this->_ptr = in_rhs._ptr;
    return *this;
}

template<typename Type, typename DataT>
MBASE_INLINE reverse_bi_list_iterator<Type, DataT> reverse_bi_list_iterator<Type, DataT>::operator-(difference_type in_rhs) noexcept
{
    reverse_bi_list_iterator bli(*this);
    bli -= in_rhs;
    return bli;
}

template<typename Type, typename DataT>
MBASE_INLINE reverse_bi_list_iterator<Type, DataT>& reverse_bi_list_iterator<Type, DataT>::operator-=(difference_type in_rhs) noexcept
{
    for (size_type i = 0; i < in_rhs; i++)
    {
        this->_ptr = this->_ptr->next;
    }
    return *this;
}

template<typename Type, typename DataT>
MBASE_INLINE reverse_bi_list_iterator<Type, DataT>& reverse_bi_list_iterator<Type, DataT>::operator--() noexcept
{
    this->_ptr = this->_ptr->next;
    return *this;
}

template<typename Type, typename DataT>
MBASE_INLINE reverse_bi_list_iterator<Type, DataT> reverse_bi_list_iterator<Type, DataT>::operator--(int) noexcept
{
    reverse_bi_list_iterator rbli(*this);
    --(*this);
    return rbli;
}

/* <-- CONST REVERSE BIDIRECTIONAL LIST ITERATOR IMPLEMENTATION -->*/
template<typename Type, typename DataT>
const_reverse_bi_list_iterator<Type, DataT>::const_reverse_bi_list_iterator() noexcept : const_backward_list_iterator<Type, DataT>()
{
}

template<typename Type, typename DataT>
const_reverse_bi_list_iterator<Type, DataT>::const_reverse_bi_list_iterator(pointer in_ptr) noexcept : const_backward_list_iterator<Type, DataT>(in_ptr) 
{
}

template<typename Type, typename DataT>
const_reverse_bi_list_iterator<Type, DataT>::const_reverse_bi_list_iterator(const const_reverse_bi_list_iterator& in_rhs) noexcept : const_backward_list_iterator<Type, DataT>(in_rhs._ptr) 
{
}

template<typename Type, typename DataT>
const_reverse_bi_list_iterator<Type, DataT>::const_reverse_bi_list_iterator(const reverse_bi_list_iterator<Type, DataT>& in_rhs) noexcept : const_backward_list_iterator<Type, DataT>(in_rhs.get()) 
{
}

template<typename Type, typename DataT>
const_reverse_bi_list_iterator<Type, DataT>& const_reverse_bi_list_iterator<Type, DataT>::operator=(const const_reverse_bi_list_iterator& in_rhs) noexcept
{
    this->_ptr = in_rhs._ptr;
    return *this;
}

template<typename Type, typename DataT>
const_reverse_bi_list_iterator<Type, DataT>& const_reverse_bi_list_iterator<Type, DataT>::operator=(const reverse_bi_list_iterator<Type, DataT>& in_rhs) noexcept
{
    this->_ptr = in_rhs._ptr;
    return *this;
}

template<typename Type, typename DataT>
MBASE_INLINE const_reverse_bi_list_iterator<Type, DataT> const_reverse_bi_list_iterator<Type, DataT>::operator-(difference_type in_rhs) noexcept
{
    const_reverse_bi_list_iterator bli(*this);
    bli -= in_rhs;
    return bli;
}

template<typename Type, typename DataT>
MBASE_INLINE const_reverse_bi_list_iterator<Type, DataT>& const_reverse_bi_list_iterator<Type, DataT>::operator-=(difference_type in_rhs) noexcept
{
    for (size_type i = 0; i < in_rhs; i++)
    {
        this->_ptr = this->_ptr->next;
    }
    return *this;
}

template<typename Type, typename DataT>
MBASE_INLINE typename const_reverse_bi_list_iterator<Type, DataT>::const_pointer const_reverse_bi_list_iterator<Type, DataT>::get() const noexcept
{
    return this->_ptr;
}

template<typename Type, typename DataT>
MBASE_INLINE const_reverse_bi_list_iterator<Type, DataT>& const_reverse_bi_list_iterator<Type, DataT>::operator--() noexcept
{
    this->_ptr = this->_ptr->next;
    return *this;
}

template<typename Type, typename DataT>
MBASE_INLINE const_reverse_bi_list_iterator<Type, DataT> const_reverse_bi_list_iterator<Type, DataT>::operator--(int) noexcept 
{
    const_reverse_bi_list_iterator cbli(*this);
    --(*this);
    return cbli;
}

MBASE_STD_END

#endif // MBASE_LIST_ITERATOR_H

