#include "core/io/dir_access.h"

#include <nn/nn_Log.h>
#include <nn/os.h>
#include <nn/fs.h>

class DirAccessNX : public DirAccess {
    nn::fs::DirectoryHandle directoryHandle;
    String current_dir;
	bool _cisdir;
	bool _cishidden;
    int index;
    Vector<nn::fs::DirectoryEntry *> dir_entries;
public:
	Error list_dir_begin() override; ///< This starts dir listing
	String get_next() override;
	bool current_is_dir() const override;
	bool current_is_hidden() const override;

	void list_dir_end() override;

	int get_drive_count() override;
	String get_drive(int p_drive) override;

	Error change_dir(String p_dir) override; ///< can be relative or absolute, return false on success
	String get_current_dir(bool p_include_drive = true) const override; ///< return current dir location
	Error make_dir(String p_dir) override;

	bool file_exists(String p_file) override;
	bool dir_exists(String p_dir) override;

	Error rename(String p_path, String p_new_path) override;
	Error remove(String p_path) override;

	bool is_link(String p_file) override;
	String read_link(String p_file) override;
	Error create_link(String p_source, String p_target) override;

	uint64_t get_space_left() override;

	String get_filesystem_type() const override;

	DirAccessNX();
	~DirAccessNX() override;
};