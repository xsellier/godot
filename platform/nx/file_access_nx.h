#include "core/io/file_access.h"

#include <nn/nn_Log.h>
#include <nn/os.h>
#include <nn/fs.h>

#define NX_CACHE_SIZE 8192

class FileAccessNX : public FileAccess {

    nn::fs::FileHandle f;
    int flags;
    mutable Error last_error;
	String path;
	String path_src;
	bool is_writable_user_data;
	bool is_writable_cache_data;
    mutable int64_t offset;
	mutable int64_t nx_file_size;
	mutable uint8_t nx_cache[NX_CACHE_SIZE];
	mutable int64_t nx_cache_start_pos;
	mutable int64_t nx_cache_end_pos;

    void _close(); ///< close a file

protected:
    Error open_internal(const String &p_path, int p_mode_flags) override; ///< open a file
    uint64_t _get_modified_time(const String &p_file) override;

public:
	uint32_t _get_unix_permissions(const String &p_file) override;
	Error _set_unix_permissions(const String &p_file, uint32_t p_permissions) override;

    bool is_open() const override; ///< true when file is open
    String get_path() const override; /// returns the path for the current open file
	String get_path_absolute() const override; /// returns the absolute path for the current open file

	void seek(uint64_t p_position) override; ///< seek to a given position
	void seek_end(int64_t p_position = 0) override; ///< seek from the end of file
	uint64_t get_position() const override; ///< get position in the file
	uint64_t get_length() const override; ///< get size of the file

	bool eof_reached() const override; ///< reading passed EOF

	uint8_t get_8() const override; ///< get a byte
	uint64_t get_buffer(uint8_t *p_dst, uint64_t p_length) const override;

	Error get_error() const override; ///< get last error

	void flush() override;
	void store_8(uint8_t p_dest) override; ///< store a byte
	void store_buffer(const uint8_t *p_src, uint64_t p_length) override; ///< store an array of bytes

	void close() override; ///< close a file

	bool file_exists(const String &p_path) override; ///< return true if a file exists



	FileAccessNX();
	~FileAccessNX() override;
};