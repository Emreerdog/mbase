#ifndef MBASE_QUEUE_H
#define MBASE_QUEUE_H

#include <mbase/common.h>
#include <mbase/list.h> // mbase::list

MBASE_STD_BEGIN

/*

	--- CLASS INFORMATION ---
Identification: S0C32-OBJ-NA-ST

Name: queue

Parent: None

Behaviour List:
- Default Constructible
- Copy Constructible
- Move Constructible
- Templated
- Type Aware

Description:

*/


template<typename T, typename SourceContainer = mbase::list<T>>
class queue {
public:
	using value_type = T;
	using size_type = SIZE_T;
	using difference_type = PTRDIFF;
	using reference = value_type&;
	using const_reference = const T&;
	using move_reference = T&&;
	using pointer = T*;
	using const_pointer = const T*;

	/* ===== BUILDER METHODS BEGIN ===== */
	MBASE_INLINE queue() noexcept;
	MBASE_INLINE queue(const queue& in_rhs) noexcept;
	MBASE_INLINE queue(queue&& in_rhs) noexcept;
	/* ===== BUILDER METHODS END ===== */

	/* ===== OPERATOR BUILDER METHODS BEGIN ===== */
	MBASE_INLINE queue& operator=(const queue& in_rhs) noexcept;
	MBASE_INLINE queue& operator=(queue&& in_rhs) noexcept;
	/* ===== OPERATOR BUILDER METHODS END ===== */

	/* ===== OBSERVATION METHODS BEGIN ===== */
	MBASE_ND(MBASE_RESULT_IGNORE) MBASE_INLINE_EXPR size_type get_serialized_size() const noexcept;
	MBASE_ND(MBASE_OBS_IGNORE) MBASE_INLINE_EXPR reference front() noexcept;
	MBASE_ND(MBASE_OBS_IGNORE) MBASE_INLINE_EXPR const_reference front() const noexcept;
	MBASE_ND(MBASE_OBS_IGNORE) MBASE_INLINE_EXPR bool empty() const noexcept;
	MBASE_ND(MBASE_OBS_IGNORE) MBASE_INLINE_EXPR size_type size() const noexcept;
	MBASE_ND(MBASE_OBS_IGNORE) SourceContainer& getHandler();
	/* ===== OBSERVATION METHODS END ===== */

	/* ===== STATE-MODIFIER METHODS BEGIN ===== */
	MBASE_INLINE_EXPR GENERIC push(const T& in_data) noexcept;
	MBASE_INLINE_EXPR GENERIC push(T&& in_data) noexcept;
	MBASE_INLINE_EXPR GENERIC pop() noexcept;
	MBASE_INLINE GENERIC deserialize(IBYTEBUFFER in_buffer, SIZE_T in_length, SIZE_T& bytes_processed);
	/* ===== STATE-MODIFIER METHODS END ===== */

	/* ===== NON-MODIFIER METHODS BEGIN ===== */
	MBASE_INLINE GENERIC serialize(char_stream& out_buffer) noexcept;
	/* ===== NON-MODIFIER METHODS END ===== */

private:
	SourceContainer _Sc;
};

template<typename T, typename SourceContainer>
MBASE_INLINE queue<T, SourceContainer>::queue() noexcept {}

template<typename T, typename SourceContainer>
MBASE_INLINE queue<T, SourceContainer>::queue(const queue& in_rhs) noexcept 
{
	_Sc = in_rhs._Sc;
}

template<typename T, typename SourceContainer>
MBASE_INLINE queue<T, SourceContainer>::queue(queue&& in_rhs) noexcept 
{
	_Sc = std::move(in_rhs._Sc);
}

template<typename T, typename SourceContainer>
MBASE_INLINE queue<T, SourceContainer>& queue<T, SourceContainer>::operator=(const queue& in_rhs) noexcept 
{
	_Sc = in_rhs._Sc;
	return *this;
}

template<typename T, typename SourceContainer>
MBASE_INLINE queue<T, SourceContainer>& queue<T, SourceContainer>::operator=(queue&& in_rhs) noexcept 
{
	_Sc = std::move(in_rhs._Sc);
	return *this;
}

template<typename T, typename SourceContainer>
MBASE_ND(MBASE_RESULT_IGNORE) MBASE_INLINE_EXPR typename queue<T, SourceContainer>::size_type queue<T, SourceContainer>::get_serialized_size() const noexcept
{
	return _Sc.get_serialized_size();
}

template<typename T, typename SourceContainer>
MBASE_ND(MBASE_OBS_IGNORE) MBASE_INLINE_EXPR typename queue<T, SourceContainer>::reference queue<T, SourceContainer>::front() noexcept
{
	return _Sc.front();
}

template<typename T, typename SourceContainer>
MBASE_ND(MBASE_OBS_IGNORE) MBASE_INLINE_EXPR typename queue<T, SourceContainer>::const_reference queue<T, SourceContainer>::front() const noexcept
{
	return _Sc.front();
}

template<typename T, typename SourceContainer>
MBASE_ND(MBASE_OBS_IGNORE) MBASE_INLINE_EXPR bool queue<T, SourceContainer>::empty() const noexcept
{
	return _Sc.empty();
}

template<typename T, typename SourceContainer>
MBASE_ND(MBASE_OBS_IGNORE) MBASE_INLINE_EXPR typename queue<T, SourceContainer>::size_type queue<T, SourceContainer>::size() const noexcept
{
	return _Sc.size();
}

template<typename T, typename SourceContainer>
MBASE_ND(MBASE_OBS_IGNORE) SourceContainer& queue<T, SourceContainer>::getHandler()
{
	return _Sc;
}

template<typename T, typename SourceContainer>
MBASE_INLINE_EXPR GENERIC queue<T, SourceContainer>::push(const T& in_data) noexcept 
{
	_Sc.push_back(in_data);
}

template<typename T, typename SourceContainer>
MBASE_INLINE_EXPR GENERIC queue<T, SourceContainer>::push(T&& in_data) noexcept 
{
	_Sc.push_back(std::move(in_data));
}

template<typename T, typename SourceContainer>
MBASE_INLINE_EXPR GENERIC queue<T, SourceContainer>::pop() noexcept 
{
	_Sc.pop_front();
}

template<typename T, typename SourceContainer>
MBASE_INLINE GENERIC queue<T, SourceContainer>::deserialize(IBYTEBUFFER in_buffer, SIZE_T in_length, SIZE_T& bytes_processed)
{
	_Sc = _Sc.deserialize(in_buffer, in_length, bytes_processed);
}

template<typename T, typename SourceContainer>
MBASE_INLINE GENERIC queue<T, SourceContainer>::serialize(char_stream& out_buffer) noexcept
{
	_Sc.serialize(out_buffer);
}

MBASE_STD_END

#endif // MBASE_QUEUE_H