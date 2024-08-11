#ifndef MBASE_PC_STATE_H
#define MBASE_PC_STATE_H

#include <mbase/common.h>
#include <mbase/behaviors.h>
#include <mbase/string.h>
#include <mbase/unordered_map.h>

MBASE_BEGIN

class MBASE_API PcState : public mbase::non_copyable {
public:
	using key_val_map = mbase::unordered_map<mbase::string, mbase::deep_char_stream>;

	enum class flags : U8 {
		STATE_SUCCESS,
		STATE_ERR_MISSING_KEY,
		STATE_ERR_MISSING_DATA,
		STATE_ERR_NOT_FOUND,
		STATE_ERR_FORMAT_IS_CORRUPT
	};

	PcState() = default;
	~PcState() = default;

	flags set_state_file(const mbase::string& in_path, bool in_transfer_state = true) noexcept { return flags::STATE_SUCCESS; }
	flags set_state_file_prefix(const mbase::string& in_prefix) { return flags::STATE_SUCCESS; }
	flags set_state(const mbase::string& in_key, char_stream& in_stream) { return flags::STATE_SUCCESS; }
	flags set_state_map(const key_val_map& in_kvmap) { return flags::STATE_SUCCESS; }
	flags load_state_from_file(const mbase::string& in_path) { return flags::STATE_SUCCESS; }
	flags write_state_to_file(const mbase::string& in_path) { return flags::STATE_SUCCESS; }
	flags update() { return flags::STATE_SUCCESS; }
	flags get_state(const mbase::string& in_key, char_stream& out_stream) { return flags::STATE_SUCCESS; }
	flags get_state_file(mbase::string& out_file) { return flags::STATE_SUCCESS; }

private:
	key_val_map mKvMap;
};

MBASE_END

#endif // !MBASE_PC_STATE_H
