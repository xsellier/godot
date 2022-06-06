#include "dir_access_nx.h"

Error DirAccessNX::list_dir_begin()
{
    list_dir_end();

    nn::Result result = nn::fs::OpenDirectory(&directoryHandle, current_dir.utf8().get_data(), nn::fs::OpenDirectoryMode_All);
    if( nn::fs::ResultPathNotFound::Includes(result) )
        return ERR_CANT_OPEN;

    int64_t entryCount;
    nn::fs::GetDirectoryEntryCount(&entryCount, directoryHandle);
    dir_entries.resize(entryCount);

    int64_t entriesStored;
    nn::fs::ReadDirectory(&entriesStored, (nn::fs::DirectoryEntry *)dir_entries.ptrw(), directoryHandle, entryCount);

    if (entryCount != entriesStored)
        return ERR_INVALID_DATA;

    return OK;
}
	
String DirAccessNX::get_next()
{
    nn::fs::DirectoryEntry *entry = dir_entries[index];
    String entryName = entry->name;
    _cishidden = (entryName != "." && entryName != ".." && entryName.begins_with("."));

    if (entry->directoryEntryType == 0)
        _cisdir = true;
    else
        _cisdir = false;
    
    index++;
    return entryName;
}

bool DirAccessNX::current_is_dir() const
{
    return _cisdir;
}

bool DirAccessNX::current_is_hidden() const
{
    return _cishidden;
}

void DirAccessNX::list_dir_end()
{
	if (directoryHandle.handle)
		nn::fs::CloseDirectory(directoryHandle);
    dir_entries.clear();
    index = 0;
    _cisdir = false;
    _cishidden = false;
    directoryHandle.handle = NULL;
}

int DirAccessNX::get_drive_count()
{
    return 2;
}
	
String DirAccessNX::get_drive(int p_drive)
{
    if (p_drive == 0)
        return ":res//";
    if (p_drive == 1)
        return ":user//";

    return "";
}

Error DirAccessNX::change_dir(String p_dir)
{
    //p_dir = fix_path(p_dir);

    // try_dir is the directory we are trying to change into
	String try_dir = "";
	if (p_dir.is_relative_path()) {
		String next_dir = current_dir + "/" + p_dir;
		next_dir = next_dir.simplify_path();
		try_dir = next_dir;
	} else {
		try_dir = p_dir;
	}

    if (!dir_exists(try_dir))
        return ERR_INVALID_PARAMETER;

    current_dir = try_dir;
    return list_dir_begin();

}

String DirAccessNX::get_current_dir(bool p_include_drive) const
{
	String base = _get_root_path();
	if (base != "") {

		String bd = current_dir.replace_first(base, "");
		if (bd.begins_with("/"))
			return _get_root_string() + bd.substr(1, bd.length());
		else
			return _get_root_string() + bd;
	}
	return current_dir;
}

Error DirAccessNX::make_dir(String p_dir)
{
    if (p_dir.is_relative_path())
		p_dir = get_current_dir().path_join(p_dir);

	//p_dir = fix_path(p_dir);

    nn::Result result = nn::fs::CreateDirectory(p_dir.utf8().get_data());
    if (nn::fs::ResultPathNotFound().Includes(result)) {
        return ERR_DOES_NOT_EXIST;
    } else if (nn::fs::ResultPathAlreadyExists().Includes(result)) {
        return ERR_ALREADY_EXISTS;
    } else if (nn::fs::ResultUsableSpaceNotEnough().Includes(result)) {
        return ERR_CANT_CREATE;
    }

    return OK;
}

bool DirAccessNX::file_exists(String p_file)
{
    if (p_file.is_relative_path())
		p_file = current_dir.path_join(p_file);

	//p_file = fix_path(p_file);

    nn::fs::DirectoryEntryType directoryEntryType;
    nn::Result result = nn::fs::GetEntryType(&directoryEntryType, p_file.utf8().get_data());
    return !nn::fs::ResultPathNotFound().Includes(result);
}

bool DirAccessNX::dir_exists(String p_dir)
{
    if (p_dir.is_relative_path())
		p_dir = get_current_dir().path_join(p_dir);

	//p_dir = fix_path(p_dir);

    nn::fs::DirectoryEntryType directoryEntryType;
    nn::Result result = nn::fs::GetEntryType(&directoryEntryType, p_dir.utf8().get_data());
    return !nn::fs::ResultPathNotFound().Includes(result);
}

Error DirAccessNX::rename(String p_path, String p_new_path)
{
	if (p_path.is_relative_path())
		p_path = get_current_dir().path_join(p_path);

	//p_path = fix_path(p_path);

	if (p_new_path.is_relative_path())
		p_new_path = get_current_dir().path_join(p_new_path);

	//p_new_path = fix_path(p_new_path);

    nn::fs::DirectoryEntryType directoryEntryType;
    nn::Result result = nn::fs::GetEntryType(&directoryEntryType, p_path.utf8().get_data());
    if (!nn::fs::ResultPathNotFound().Includes(result)) {
        if (directoryEntryType == nn::fs::DirectoryEntryType_Directory) {
            // rename directory
            result = nn::fs::RenameDirectory(p_path.utf8().get_data(), p_new_path.utf8().get_data());
        } else if (directoryEntryType == nn::fs::DirectoryEntryType_File) {
            // rename file
            result = nn::fs::RenameFile(p_path.utf8().get_data(), p_new_path.utf8().get_data());
        }
        if (nn::fs::ResultPathNotFound().Includes(result)) {
            return ERR_DOES_NOT_EXIST;
        } else if (nn::fs::ResultAlreadyExists().Includes(result)) {
            return ERR_ALREADY_EXISTS;
        } else if (nn::fs::ResultTargetLocked().Includes(result)) {
            return ERR_FILE_ALREADY_IN_USE;
        }
    } else {
        return ERR_FILE_NOT_FOUND;
    }

    return OK;
}

Error DirAccessNX::remove(String p_path)
{
    if (p_path.is_relative_path())
		p_path = get_current_dir().path_join(p_path);

	//p_path = fix_path(p_path);

    nn::fs::DirectoryEntryType directoryEntryType;
    nn::Result result = nn::fs::GetEntryType(&directoryEntryType, p_path.utf8().get_data());
    if (!nn::fs::ResultPathNotFound().Includes(result)) {
        if (directoryEntryType == nn::fs::DirectoryEntryType_Directory) {
            // remove directory
            result = nn::fs::DeleteDirectory(p_path.utf8().get_data());
        } else if (directoryEntryType == nn::fs::DirectoryEntryType_File) {
            // remove file
            result = nn::fs::DeleteFile(p_path.utf8().get_data());
        }
        if (nn::fs::ResultPathNotFound().Includes(result)) {
            return ERR_DOES_NOT_EXIST;
        } else if (nn::fs::ResultTargetLocked().Includes(result)) {
            return ERR_FILE_ALREADY_IN_USE;
        }
    } else {
        return ERR_FILE_NOT_FOUND;
    }
    return OK;
}

bool DirAccessNX::is_link(String p_file)
{
    return false;
}

String DirAccessNX::read_link(String p_file)
{
    return p_file;
}

Error DirAccessNX::create_link(String p_source, String p_target)
{
    return FAILED;
}

uint64_t DirAccessNX::get_space_left()
{
    return 0;
}   

String DirAccessNX::get_filesystem_type() const
{
    return "";
}

DirAccessNX::DirAccessNX()
{
    directoryHandle.handle = nullptr;
    index = 0;
}

DirAccessNX::~DirAccessNX()
{

}