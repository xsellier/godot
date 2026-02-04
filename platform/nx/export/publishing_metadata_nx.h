#ifndef PUBLISHING_METADATA_NX_H
#define PUBLISHING_METADATA_NX_H

#include "core/io/resource.h"

class LocalizationMetadataNX : public Resource {
	GDCLASS(LocalizationMetadataNX, Resource);

	String language;
	String name;
	String publisher;
	String icon_path;

protected:
	static void _bind_methods();

public:
	void set_language(const String &p_language);
	String get_language() const;
	static bool is_valid_language(const String &p_language);

	void set_application_name(const String &p_name);
	String get_application_name() const;

	void set_publisher(const String &p_publisher);
	String get_publisher() const;

	void set_icon_path(const String &p_icon_path);
	String get_icon_path() const;
};

class RatingMetadataNX : public Resource {
	GDCLASS(RatingMetadataNX, Resource);

	String organization;
	int age = 0;

protected:
	static void _bind_methods();

public:
	void set_organization(const String &p_organization);
	String get_organization() const;
	static bool is_valid_organization(const String &p_organization);

	void set_age(int p_age);
	int get_age() const;
};

class PublishingMetadataNX : public Resource {
	GDCLASS(PublishingMetadataNX, Resource);
	
	String application_id = "0x01004b9000490000";
	int release_version = 0;
	String display_version = "1.0.0";
	bool demo = false;
	String legal_information;
	String accessible_urls;
	Array localization_metadata;
	Array rating_metadata;
	String appropriate_age_for_china = "";

	/* ADVANCED OPTIONS */
	
	String save_data_size = "0x0000000000400000";
	String save_data_journal_size = "0x0000000000100000";
	bool data_loss_confirmation_required = false;

protected:
	static void _bind_methods();

public:
	void set_application_id(const String &p_application_id);
	String get_application_id() const;
	
	void set_release_version(int p_release_version);
	int get_release_version() const;
	
	void set_display_version(const String &p_display_version);
	String get_display_version() const;

	void set_demo(bool p_demo);
	bool is_demo() const;

	void set_legal_information(const String &p_legal_information);
	String get_legal_information() const;

	void set_accessible_urls(const String &p_accessible_urls);
	String get_accessible_urls() const;

	void set_localization_metadata(const Array &p_localization_metadata);
	Array get_localization_metadata() const;
	int get_localization_count() const;
	Ref<LocalizationMetadataNX> get_localization(int p_index) const;

	void set_rating_metadata(const Array &p_rating_metadata);
	Array get_rating_metadata() const;
	int get_rating_count() const;
	Ref<RatingMetadataNX> get_rating(int p_index) const;
	bool has_rating_metadata() const;
	
	void set_appropriate_age_for_china(const String &p_age_for_china);
	String get_appropriate_age_for_china() const;
	static bool is_valid_age_for_china(const String &p_age_for_china);

	void set_save_data_size(const String &p_save_data_size);
	String get_save_data_size() const;

	void set_save_data_journal_size(const String &p_save_data_journal_size);
	String get_save_data_journal_size() const;

	void set_data_loss_confirmation_required(bool p_data_loss_confirmation_required);
	bool is_data_loss_confirmation_required() const;
};

#endif // PUBLISHING_METADATA_NX_H
