#ifndef MBASE_GGUF_METADATA_CONFIGURATOR
#define MBASE_GGUF_METADATA_CONFIGURATOR

#include <mbase/common.h>
#include <mbase/string.h>
#include <mbase/behaviors.h>
#include <ggml.h>
#include <unordered_map>

MBASE_BEGIN

template<typename T>
struct MBASE_API gguf_value_reader {};

template<>
struct MBASE_API gguf_value_reader<I8> {
	static I8 read(gguf_context* in_context, I32 in_key_id) { return gguf_get_val_i8(in_context, in_key_id); }
};

template<>
struct MBASE_API gguf_value_reader<I16> {
	static I16 read(gguf_context* in_context, I32 in_key_id) { return gguf_get_val_i16(in_context, in_key_id); }
};

template<>
struct MBASE_API gguf_value_reader<I32> {
	static I32 read(gguf_context* in_context, I32 in_key_id) { return gguf_get_val_i32(in_context, in_key_id); }
};

template<>
struct MBASE_API gguf_value_reader<I64> {
	static I64 read(gguf_context* in_context, I32 in_key_id) { return gguf_get_val_i64(in_context, in_key_id); }
};

template<>
struct MBASE_API gguf_value_reader<U8> {
	static U8 read(gguf_context* in_context, I32 in_key_id) { return gguf_get_val_u8(in_context, in_key_id); }
};

template<>
struct MBASE_API gguf_value_reader<U16> {
	static U16 read(gguf_context* in_context, I32 in_key_id) { return gguf_get_val_u16(in_context, in_key_id); }
};

template<>
struct MBASE_API gguf_value_reader<U32> {
	static U32 read(gguf_context* in_context, I32 in_key_id) { return gguf_get_val_u32(in_context, in_key_id); }
};

template<>
struct MBASE_API gguf_value_reader<U64> {
	static U64 read(gguf_context* in_context, I32 in_key_id) { return gguf_get_val_u64(in_context, in_key_id); }
};

template<>
struct MBASE_API gguf_value_reader<F32> {
	static F32 read(gguf_context* in_context, I32 in_key_id) { return gguf_get_val_f32(in_context, in_key_id); }
};

template<>
struct MBASE_API gguf_value_reader<F64> {
	static F64 read(gguf_context* in_context, I32 in_key_id) { return gguf_get_val_f64(in_context, in_key_id); }
};

template<>
struct MBASE_API gguf_value_reader<bool> {
	static bool read(gguf_context* in_context, I32 in_key_id) { return gguf_get_val_bool(in_context, in_key_id); }
};

template<>
struct MBASE_API gguf_value_reader<mbase::string> {
	static mbase::string read(gguf_context* in_context, I32 in_key_id) { return gguf_get_val_str(in_context, in_key_id); }
};

template<>
struct MBASE_API gguf_value_reader<const char*> {
	static const char* read(gguf_context* in_context, I32 in_key_id) { return gguf_get_val_str(in_context, in_key_id); }
};

template<typename T> struct MBASE_API gguf_value_writer{};

template<>
struct MBASE_API gguf_value_writer<I8> {
	static GENERIC write(gguf_context* in_context, const mbase::string& in_key, const I8& in_value) { return gguf_set_val_i8(in_context, in_key.c_str(), in_value); }
};

template<>
struct MBASE_API gguf_value_writer<I16> {
	static GENERIC write(gguf_context* in_context, const mbase::string& in_key, const I16& in_value) { return gguf_set_val_i16(in_context, in_key.c_str(), in_value); }
};

template<>
struct MBASE_API gguf_value_writer<I32> {
	static GENERIC write(gguf_context* in_context, const mbase::string& in_key, const I32& in_value) { return gguf_set_val_i32(in_context, in_key.c_str(), in_value); }
};

template<>
struct MBASE_API gguf_value_writer<I64> {
	static GENERIC write(gguf_context* in_context, const mbase::string& in_key, const I64& in_value) { return gguf_set_val_i64(in_context, in_key.c_str(), in_value); }
};

template<>
struct MBASE_API gguf_value_writer<U8> {
	static GENERIC write(gguf_context* in_context, const mbase::string& in_key, const U8& in_value) { return gguf_set_val_u8(in_context, in_key.c_str(), in_value); }
};

template<>
struct MBASE_API gguf_value_writer<U16> {
	static GENERIC write(gguf_context* in_context, const mbase::string& in_key, const U16& in_value) { return gguf_set_val_u16(in_context, in_key.c_str(), in_value); }
};

template<>
struct MBASE_API gguf_value_writer<U32> {
	static GENERIC write(gguf_context* in_context, const mbase::string& in_key, const U32& in_value) { return gguf_set_val_u32(in_context, in_key.c_str(), in_value); }
};

template<>
struct MBASE_API gguf_value_writer<U64> {
	static GENERIC write(gguf_context* in_context, const mbase::string& in_key, const U64& in_value) { return gguf_set_val_u64(in_context, in_key.c_str(), in_value); }
};

template<>
struct MBASE_API gguf_value_writer<F32> {
	static GENERIC write(gguf_context* in_context, const mbase::string& in_key, const F32& in_value) { return gguf_set_val_f32(in_context, in_key.c_str(), in_value); }
};

template<>
struct MBASE_API gguf_value_writer<F64> {
	static GENERIC write(gguf_context* in_context, const mbase::string& in_key, const F64& in_value) { return gguf_set_val_f64(in_context, in_key.c_str(), in_value); }
};

template<>
struct MBASE_API gguf_value_writer<bool> {
	static GENERIC write(gguf_context* in_context, const mbase::string& in_key, const bool& in_value) { return gguf_set_val_bool(in_context, in_key.c_str(), in_value); }
};

template<>
struct MBASE_API gguf_value_writer<mbase::string> {
	static GENERIC write(gguf_context* in_context, const mbase::string& in_key, const mbase::string& in_value) { return gguf_set_val_str(in_context, in_key.c_str(), in_value.c_str()); }
};

template<>
struct MBASE_API gguf_value_writer<const char*> {
	static GENERIC write(gguf_context* in_context, const mbase::string& in_key, const char* in_value) { return gguf_set_val_str(in_context, in_key.c_str(), in_value); }
};

class MBASE_API GgufMetaConfigurator : public mbase::non_copymovable {
public:
	using kv_map = std::unordered_map<mbase::string, I32>;
	using size_type = SIZE_T;

	enum class purification_error : U8 {
		SUCCESS,
		ERR_ARCH_NOT_SET,
		ERR_BLOCK_COUNT_NOT_FOUND,
		ERR_EMBEDDING_LENGTH_NOT_FOUND,
		ERR_ATTENTION_HEAD_COUNT_NOT_FOUND
	};

	GgufMetaConfigurator(const mbase::wstring in_filename);
	~GgufMetaConfigurator();

	template<typename T>
	bool get_key(const mbase::string& in_key, T& out_value)
	{
		if(!is_open())
		{
			return false;
		}

		if(mMetadataMap.find(in_key) == mMetadataMap.end() || !mGgufContext)
		{
			return false;
		}

		out_value = mbase::gguf_value_reader<T>::read(mGgufContext, mMetadataMap[in_key]);
		return true;
	}

	bool is_open();
	size_type get_metadata_count();
	kv_map& get_kv_map();

	template<typename T>
	GENERIC set_key(const mbase::string& in_key, const T& in_value)
	{
		if(is_open())
		{
			mbase::gguf_value_writer<T>::write(mGgufContext, in_key, in_value);
			mIsModified = true;
		}
	}
	GENERIC remove_key(const mbase::string& in_key);
	purification_error apply_purification(const mbase::string& in_system_prompt);

private:
	GENERIC _clear_context();
	gguf_context* mGgufContext;
	kv_map mMetadataMap;
	bool mIsModified;
	mbase::wstring mGgufFile;
	size_type mOldMetaSize;
};

MBASE_END

#endif // !MBASE_GGUF_METADATA_CONFIGURATOR
