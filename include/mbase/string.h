#ifndef MBASE_STRING_H
#define MBASE_STRING_H

#include <mbase/type_sequence.h>

MBASE_STD_BEGIN

class character_sequence : private type_sequence<IBYTE> {
public:
    using Alloc = mbase::allocator_simple<IBYTE>;

    character_sequence() noexcept : raw_data(nullptr), mSize(0), mCapacity(8) {
        raw_data = Alloc::allocate(mCapacity, true);
    }

    character_sequence(IBYTEBUFFER in_data, size_type in_size, size_type in_capacity) noexcept : raw_data(in_data), mSize(in_size), mCapacity(in_capacity) {}

    character_sequence(MSTRING in_string) noexcept {
        size_type tmp_st_length = type_sequence::length(in_string);
        size_type string_length = tmp_st_length / 8;
        size_type base_capacity = 8;
        while (string_length--)
        {
            // calculating capacity
            base_capacity *= 2;
        }
        mCapacity = base_capacity;
        mSize = tmp_st_length;
        raw_data = Alloc::allocate(base_capacity, true);
        type_sequence::copy(raw_data, in_string, tmp_st_length + 1);
    }

    character_sequence(MSTRING in_string, size_type in_length) noexcept {
        size_type string_length = in_length / 8;
        size_type base_capacity = 8;
        while (string_length--)
        {
            // calculating capacity
            base_capacity *= 2;
        }
        mCapacity = base_capacity;
        mSize = in_length;
        raw_data = Alloc::allocate(base_capacity, true);
        type_sequence::copy(raw_data, in_string, in_length);
    }

    character_sequence(const character_sequence& in_rhs) noexcept : raw_data(nullptr), mSize(in_rhs.mSize), mCapacity(in_rhs.mCapacity) {
        raw_data = Alloc::allocate(mCapacity, true);
        type_sequence::copy(raw_data, in_rhs.raw_data, mSize);
    }

    character_sequence(character_sequence&& in_rhs) noexcept : raw_data(in_rhs.raw_data), mSize(in_rhs.mSize), mCapacity(in_rhs.mCapacity) {
        in_rhs.raw_data = NULL;
    }

    ~character_sequence() noexcept {
        Alloc::destroy(raw_data);
    }

    character_sequence& operator=(const character_sequence& in_rhs) noexcept {
        if(raw_data)
        {
            Alloc::deallocate(raw_data);
        }

        mSize = in_rhs.mSize;
        mCapacity = in_rhs.mCapacity;

        raw_data = Alloc::allocate(in_rhs.mCapacity, true);
        type_sequence::copy(raw_data, in_rhs.raw_data, in_rhs.mSize);
        return *this;
    }

    character_sequence& operator=(MSTRING in_rhs) noexcept {
        if (raw_data)
        {
            Alloc::deallocate(raw_data);
        }

        size_type st_length = type_sequence::length(in_rhs);
        mSize = st_length;
        mCapacity = 8; // Base capacity
        
        while(mCapacity <= st_length)
        {
            // grow until the capacity exceeds the string
            mCapacity *= 2;
        }
        
        raw_data = Alloc::allocate(mCapacity, true);
        type_sequence::copy(raw_data, in_rhs, st_length);

        return *this;
    }

    character_sequence& operator=(character_sequence&& in_rhs) noexcept {
        if (raw_data)
        {
            Alloc::deallocate(raw_data);
        }

        mSize = in_rhs.mSize;
        mCapacity = in_rhs.mCapacity;
        raw_data = in_rhs.raw_data;
        in_rhs.raw_data = nullptr;
        return *this;
    }

    character_sequence& operator+=(const character_sequence& in_rhs) noexcept {
        if(!in_rhs.mSize)
        {
            return *this;
        }

        size_type totalSize = mSize + in_rhs.mSize;
        if (totalSize >= mCapacity)
        {
           while(totalSize >= mCapacity)
           {
               mCapacity *= 2;
           }
           _resize(mCapacity);
        }

        type_sequence::concat(raw_data + mSize, in_rhs.raw_data, in_rhs.mSize);
        mSize = totalSize;
        return *this;
    }

    character_sequence& operator+=(MSTRING in_rhs) noexcept {
        if(!in_rhs)
        {
            return *this;
        }
        size_type rhsSize = type_sequence::length(in_rhs);
        size_type totalSize = mSize + rhsSize;
        if(totalSize == mSize)
        {
            return *this;
        }

        if(totalSize >= mCapacity)
        {
            while(totalSize >= mCapacity)
            {
                mCapacity *= 2;
            }
            _resize(mCapacity);
        }
        
        type_sequence::concat(raw_data + mSize, in_rhs, rhsSize);
        mSize = totalSize;
        return *this;
    }

    character_sequence& operator+=(const IBYTE& in_character) noexcept {
        mSize += 1;
        if(mSize >= mCapacity)
        {
            _resize(++mCapacity);
        }
        raw_data[mSize-1] = in_character;
        return *this;
    }

    USED_RETURN MBASE_INLINE size_type find(MSTRING in_src, size_type in_pos = 0) const noexcept {
        MSTRING tmpResult = strstr(raw_data + in_pos, in_src);
        if (tmpResult)
        {
            return tmpResult - (raw_data + in_pos);
        }

        return npos;
    }

    USED_RETURN MBASE_INLINE size_type find(const character_sequence& in_src, size_type in_pos = 0) const noexcept {
        return find(in_src.c_str(), in_pos);
    }

    USED_RETURN MBASE_INLINE size_type find(IBYTE in_char, size_type in_pos = 0) const noexcept {
        MSTRING tmpResult = strchr(raw_data + in_pos, in_char);
        if(tmpResult)
        {
            return tmpResult - (raw_data + in_pos);
        }

        return npos;
    }

    template<typename ... Params>
    USED_RETURN static character_sequence from_format(MSTRING in_format, Params ... in_params) noexcept {
        I32 stringLength = _scprintf(in_format, std::forward<Params>(in_params)...); // FIND THE FKIN SIZE
        character_sequence newSequence;
        if(!stringLength)
        {
            return newSequence;
        }

        IBYTEBUFFER mString = new IBYTE[stringLength + 1];
        fill(mString, 0, stringLength + 1);
        sprintf(mString, in_format, std::forward<Params>(in_params)...);
        newSequence = std::move(character_sequence(mString));
        delete[]mString;

        return newSequence;
    }

    USED_RETURN static I32 to_i32(const character_sequence& in_string) noexcept {
        return atoi(in_string.c_str());
    }

    USED_RETURN static I32 to_i64(const character_sequence& in_string) noexcept {
        return _atoi64(in_string.c_str());
    }

    USED_RETURN static F32 to_f32(const character_sequence& in_string) noexcept {
        return strtof(in_string.c_str(), nullptr);
    }

    USED_RETURN static F64 to_f64(const character_sequence& in_string) noexcept {
        return atof(in_string.c_str());
    }

    USED_RETURN MBASE_INLINE I32 to_i32() const noexcept {
        return atoi(raw_data);
    }

    USED_RETURN MBASE_INLINE I64 to_i64() const noexcept {
        return _atoi64(raw_data);
    }

    USED_RETURN MBASE_INLINE F32 to_f32() const noexcept {
        return strtof(raw_data, nullptr);
    }

    USED_RETURN MBASE_INLINE F64 to_f64() const noexcept {
        return atof(raw_data);
    }

    USED_RETURN MBASE_INLINE GENERIC resize(size_type in_amount) {
        _resize(in_amount);
    }

    MBASE_INLINE GENERIC swap(character_sequence& in_src) noexcept {
        std::swap(raw_data, in_src.raw_data);
        std::swap(mCapacity, in_src.mCapacity);
        std::swap(mSize, in_src.mSize);
    }

    USED_RETURN MBASE_INLINE iterator begin() const noexcept {
        return iterator(raw_data);
    }

    USED_RETURN MBASE_INLINE iterator end() const noexcept {
        return iterator(raw_data + mSize);
    }

    USED_RETURN MBASE_INLINE const_iterator cbegin() const noexcept {
        return const_iterator(raw_data);
    }

    USED_RETURN MBASE_INLINE const_iterator cend() const noexcept {
        return const_iterator(raw_data + mSize);
    }

    USED_RETURN MBASE_INLINE const_pointer c_str() const noexcept {
        return raw_data;
    }

    USED_RETURN MBASE_INLINE pointer data() const noexcept {
        return raw_data;
    }

    USED_RETURN MBASE_INLINE reference front() const noexcept {
        return raw_data[0];
    }

    USED_RETURN MBASE_INLINE reference back() const noexcept {
        return raw_data[mSize - 1];
    }

    USED_RETURN MBASE_INLINE reference at(size_type in_pos) const noexcept {
        return raw_data[in_pos];
    }

    USED_RETURN MBASE_INLINE reference operator[](size_type in_pos) const noexcept {
        return raw_data[in_pos];
    }

    USED_RETURN MBASE_INLINE_EXPR size_type size() const noexcept {
        return mSize;
    }

    USED_RETURN MBASE_INLINE_EXPR size_type capacity() const noexcept {
        return mCapacity - 1;
    }

    friend character_sequence operator+(const character_sequence& in_lhs, MSTRING in_rhs) noexcept {
        size_type rhsSize = type_sequence::length(in_rhs);
        if (!rhsSize)
        {
            return character_sequence(in_lhs);
        }
        size_type totalSize = in_lhs.mSize + type_sequence::length(in_rhs);
        size_type totalCapacity = in_lhs.mCapacity;
        while (totalSize >= totalCapacity)
        {
            totalCapacity *= 2;
        }

        IBYTEBUFFER new_data = Alloc::allocate(totalCapacity, true);
        type_sequence::concat(new_data, in_lhs.raw_data, in_lhs.mSize);
        type_sequence::concat(new_data + in_lhs.mSize, in_rhs, rhsSize);

        return character_sequence(new_data, totalSize, totalCapacity);
    }

    friend character_sequence operator+(const character_sequence& in_lhs, const IBYTE& in_rhs) noexcept {
        size_type totalCapacity = in_lhs.mCapacity;
        if(in_lhs.mSize + 1 >= totalCapacity)
        {
            totalCapacity *= 2;
        }

        IBYTEBUFFER new_data = Alloc::allocate(totalCapacity, true);
        type_sequence::concat(new_data, in_lhs.raw_data, in_lhs.mSize);
        type_sequence::concat(new_data + in_lhs.mSize, (IBYTEBUFFER)&in_rhs, 1);

        return character_sequence(new_data, in_lhs.mSize + 1, totalCapacity);
    }

    friend character_sequence operator+(const character_sequence& in_lhs, const character_sequence& in_rhs) noexcept {
        size_type totalSize = in_lhs.mSize + in_rhs.mSize;
        size_type totalCapacity = in_lhs.mCapacity;

        while (totalSize >= totalCapacity)
        {
            totalCapacity *= 2;
        }
        IBYTEBUFFER new_data = Alloc::allocate(totalCapacity, true);

        type_sequence::concat(new_data, in_lhs.raw_data, in_lhs.mSize);
        type_sequence::concat(new_data + in_lhs.mSize, in_rhs.raw_data, in_rhs.mSize);

        return character_sequence(new_data, totalSize, totalCapacity);
    }

    bool operator==(const character_sequence& in_rhs) noexcept {
        return type_sequence::is_equal(raw_data, in_rhs.raw_data);
    }

    bool operator!=(const character_sequence& in_rhs) noexcept {
        return !type_sequence::is_equal(raw_data, in_rhs.raw_data);
    }

    friend bool operator==(const character_sequence& in_lhs, const character_sequence& in_rhs) noexcept {
        return type_sequence::is_equal(in_lhs.raw_data, in_rhs.raw_data);
    }

    friend bool operator!=(const character_sequence& in_lhs, const character_sequence& in_rhs) noexcept {
        return !type_sequence::is_equal(in_lhs.raw_data, in_rhs.raw_data);
    }

    friend std::ostream& operator<<(std::ostream& os, const character_sequence& in_rhs) noexcept {
        if (!in_rhs.raw_data)
        {
            // DO NOTHING IF THE DATA IS NOT PRESENT
        }
        else
        {
            os << in_rhs.data();
        }
        return os;
    }

    MBASE_INLINE GENERIC serialize(safe_buffer* out_buffer) {
        if(mSize)
        {
            out_buffer->bfLength = mSize;
            out_buffer->bfSource = new IBYTE[mSize];

            copy(out_buffer->bfSource, raw_data, mSize); // DO NOT INCLUDE NULL TERMINATOR
        }
    }

    MBASE_INLINE static character_sequence deserialize(IBYTEBUFFER in_buffer, SIZE_T in_length) noexcept {
        return mbase::character_sequence(in_buffer, in_length);
    }

    static MBASE_INLINE_EXPR const SIZE_T npos = -1;

private:

    MBASE_INLINE GENERIC _resize(SIZE_T in_size) noexcept {
        size_type expectedSize = mSize;
        if(in_size < mSize)
        {
            // We will shrink if the given size is less then our string size
            expectedSize = in_size;
        }

        IBYTEBUFFER new_data = Alloc::allocate(in_size, true);
        type_sequence::copy(new_data, raw_data, expectedSize);
        Alloc::deallocate(raw_data);
        raw_data = new_data;
        mCapacity = in_size;
        mSize = expectedSize;
    }

    IBYTEBUFFER raw_data;
    SIZE_T mCapacity;
    SIZE_T mSize;
};

using string = character_sequence;
using wstring = string; // WSTRING WILL BE IMPLEMENTED LATER
using string_view = string; // STRING VIEW WILL BE IMPLEMENTED LATER

MBASE_STD_END

#endif // MBASE_STRING_H