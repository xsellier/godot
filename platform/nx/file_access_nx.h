#include "core/io/file_access.h"

#include <nn/nn_Log.h>
#include <nn/os.h>
#include <nn/fs.h>

// #define NX_CACHE_SIZE 8192
// #define NX_CACHE_SIZE 262144
// #define NX_CACHE_SIZE 131072
// Recommended 1MB by Nintendo
#define NX_CACHE_SIZE 1048576

class FileAccessNX : public FileAccess {
	static int open_files_rw_user;
	static int open_files_rw_cache;

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
	void ensure_cache(int64_t length) const;

    void _close(); ///< close a file
	

protected:
    Error open_internal(const String &p_path, int p_mode_flags) override; ///< open a file
    uint64_t _get_modified_time(const String &p_file) override;

public:
	virtual BitField<FileAccess::UnixPermissionFlags> _get_unix_permissions(const String &p_file) override { return 0; }
	virtual Error _set_unix_permissions(const String &p_file, BitField<FileAccess::UnixPermissionFlags> p_permissions) override { return FAILED; }

	virtual bool _get_hidden_attribute(const String &p_file) override { return false; }
	virtual Error _set_hidden_attribute(const String &p_file, bool p_hidden) override { return ERR_UNAVAILABLE; }
	virtual bool _get_read_only_attribute(const String &p_file) override { return false; }
	virtual Error _set_read_only_attribute(const String &p_file, bool p_ro) override { return ERR_UNAVAILABLE; }

    bool is_open() const override; ///< true when file is open
    String get_path() const override; /// returns the path for the current open file
	String get_path_absolute() const override; /// returns the absolute path for the current open file

	void seek(uint64_t p_position) override; ///< seek to a given position
	void seek_end(int64_t p_position = 0) override; ///< seek from the end of file
	uint64_t get_position() const override; ///< get position in the file
	uint64_t get_length() const override; ///< get size of the file

	bool eof_reached() const override; ///< reading passed EOF

	virtual Error resize(int64_t p_length) override { return ERR_UNAVAILABLE; }
	uint8_t get_8() const override; ///< get a byte
	uint64_t get_buffer(uint8_t *p_dst, uint64_t p_length) const override;

	Error get_error() const override; ///< get last error

	void flush() override;
	bool store_8(uint8_t p_dest) override; ///< store a byte
	bool store_buffer(const uint8_t *p_src, uint64_t p_length) override; ///< store an array of bytes

	void close() override; ///< close a file

	bool file_exists(const String &p_path) override; ///< return true if a file exists

    virtual int64_t _get_size(const String &p_file) override;
	virtual uint64_t _get_access_time(const String &p_file) override { return 0; }

	FileAccessNX();
	~FileAccessNX() override;
};
