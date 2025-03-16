#include "publishing_metadata_nx.h"

// Taken from: https://developer.nintendo.com/html/online-docs/g1kr9vj6-en/Packages/SDK/NintendoSDK/Documents/Package/contents/Pages/Page_166503043.html
static const char *organizations_raw_string = "CERO,GRACGCRB,GSRMR,ESRB,ClassInd,USK,PEGI,PEGIBBFC,Russian,ACB,OFLC,IARCGeneric";
static const char *languages_raw_string = "AmericanEnglish,BritishEnglish,Japanese,French,German,LatinAmericanSpanish,Spanish,Italian,Dutch,CanadianFrench,Portuguese,Russian,SimplifiedChinese,TraditionalChinese,Korean,BrazilianPortuguese";
static const char *ages_for_china_raw_string = "None,Age8,Age12,Age16";

static HashSet<String> supported_organizations;
static HashSet<String> supported_languages;
static HashSet<String> supported_ages_for_china;

void LocalizationMetadataNX::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_language", "language"), &LocalizationMetadataNX::set_language);
	ClassDB::bind_method(D_METHOD("get_language"), &LocalizationMetadataNX::get_language);
	ClassDB::bind_static_method("LocalizationMetadataNX", D_METHOD("is_valid_language", "language"), &LocalizationMetadataNX::is_valid_language);
	ClassDB::bind_method(D_METHOD("set_application_name", "name"), &LocalizationMetadataNX::set_application_name);
	ClassDB::bind_method(D_METHOD("get_application_name"), &LocalizationMetadataNX::get_application_name);
	ClassDB::bind_method(D_METHOD("set_publisher", "publisher"), &LocalizationMetadataNX::set_publisher);
	ClassDB::bind_method(D_METHOD("get_publisher"), &LocalizationMetadataNX::get_publisher);
	ClassDB::bind_method(D_METHOD("set_icon_path", "icon_path"), &LocalizationMetadataNX::set_icon_path);
	ClassDB::bind_method(D_METHOD("get_icon_path"), &LocalizationMetadataNX::get_icon_path);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "language", PROPERTY_HINT_ENUM, languages_raw_string), "set_language", "get_language");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "name"), "set_application_name", "get_application_name");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "publisher"), "set_publisher", "get_publisher");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "icon_path", PROPERTY_HINT_FILE, "*.bmp,*.jpg"), "set_icon_path", "get_icon_path");

	for (const String &organization : String(organizations_raw_string).split(",")) {
		supported_organizations.insert(organization.strip_edges());
	}
	for (const String &language : String(languages_raw_string).split(",")) {
		supported_languages.insert(language.strip_edges());
	}
}

void LocalizationMetadataNX::set_language(const String &p_language) {
	language = p_language;
}

String LocalizationMetadataNX::get_language() const {
	return language;
}

bool LocalizationMetadataNX::is_valid_language(const String &p_language) {
	return supported_languages.has(p_language);
}

void LocalizationMetadataNX::set_application_name(const String &p_name) {
	name = p_name;
}

String LocalizationMetadataNX::get_application_name() const {
	return name;
}

void LocalizationMetadataNX::set_publisher(const String &p_publisher) {
	publisher = p_publisher;
}

String LocalizationMetadataNX::get_publisher() const {
	return publisher;
}

void LocalizationMetadataNX::set_icon_path(const String &p_icon_path) {
	icon_path = p_icon_path;
}

String LocalizationMetadataNX::get_icon_path() const {
	return icon_path;
}

void RatingMetadataNX::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_organization", "organization"), &RatingMetadataNX::set_organization);
	ClassDB::bind_method(D_METHOD("get_organization"), &RatingMetadataNX::get_organization);
	ClassDB::bind_static_method("RatingMetadataNX", D_METHOD("is_valid_organization", "organization"), &RatingMetadataNX::is_valid_organization);
	
	ClassDB::bind_method(D_METHOD("set_age", "age"), &RatingMetadataNX::set_age);
	ClassDB::bind_method(D_METHOD("get_age"), &RatingMetadataNX::get_age);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "organization", PROPERTY_HINT_ENUM, organizations_raw_string), "set_organization", "get_organization");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "age", PROPERTY_HINT_RANGE, "0,255,1"), "set_age", "get_age");
}

void RatingMetadataNX::set_organization(const String &p_organization) {
	organization = p_organization;
}

String RatingMetadataNX::get_organization() const {
	return organization;
}

bool RatingMetadataNX::is_valid_organization(const String &p_organization) {
	return supported_organizations.has(p_organization);
}

void RatingMetadataNX::set_age(int p_age) {
	age = p_age;
}

int RatingMetadataNX::get_age() const {
	return age;
}

void PublishingMetadataNX::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_application_id", "application_id"), &PublishingMetadataNX::set_application_id);
	ClassDB::bind_method(D_METHOD("get_application_id"), &PublishingMetadataNX::get_application_id);
	ClassDB::bind_method(D_METHOD("set_release_version", "release_version"), &PublishingMetadataNX::set_release_version);
	ClassDB::bind_method(D_METHOD("get_release_version"), &PublishingMetadataNX::get_release_version);
	ClassDB::bind_method(D_METHOD("set_display_version", "display_version"), &PublishingMetadataNX::set_display_version);
	ClassDB::bind_method(D_METHOD("get_display_version"), &PublishingMetadataNX::get_display_version);
	ClassDB::bind_method(D_METHOD("set_legal_information", "legal_information"), &PublishingMetadataNX::set_legal_information);
	ClassDB::bind_method(D_METHOD("get_legal_information"), &PublishingMetadataNX::get_legal_information);
	ClassDB::bind_method(D_METHOD("set_localization_metadata", "localization_metadata"), &PublishingMetadataNX::set_localization_metadata);
	ClassDB::bind_method(D_METHOD("get_localization_metadata"), &PublishingMetadataNX::get_localization_metadata);
	ClassDB::bind_method(D_METHOD("set_rating_metadata", "rating_metadata"), &PublishingMetadataNX::set_rating_metadata);
	ClassDB::bind_method(D_METHOD("get_rating_metadata"), &PublishingMetadataNX::get_rating_metadata);
	ClassDB::bind_method(D_METHOD("has_rating_metadata"), &PublishingMetadataNX::has_rating_metadata);
	ClassDB::bind_method(D_METHOD("set_demo", "demo"), &PublishingMetadataNX::set_demo);
	ClassDB::bind_method(D_METHOD("is_demo"), &PublishingMetadataNX::is_demo);
	ClassDB::bind_method(D_METHOD("set_appropriate_age_for_china", "age_for_china"), &PublishingMetadataNX::set_appropriate_age_for_china);
	ClassDB::bind_method(D_METHOD("get_appropriate_age_for_china"), &PublishingMetadataNX::get_appropriate_age_for_china);
	ClassDB::bind_method(D_METHOD("set_save_data_size", "save_data_size"), &PublishingMetadataNX::set_save_data_size);
	ClassDB::bind_method(D_METHOD("get_save_data_size"), &PublishingMetadataNX::get_save_data_size);
	ClassDB::bind_method(D_METHOD("set_save_data_journal_size", "save_data_journal_size"), &PublishingMetadataNX::set_save_data_journal_size);
	ClassDB::bind_method(D_METHOD("get_save_data_journal_size"), &PublishingMetadataNX::get_save_data_journal_size);
	ClassDB::bind_method(D_METHOD("set_data_loss_confirmation_required", "data_loss_confirmation_required"), &PublishingMetadataNX::set_data_loss_confirmation_required);
	ClassDB::bind_method(D_METHOD("is_data_loss_confirmation_required"), &PublishingMetadataNX::is_data_loss_confirmation_required);

	ADD_GROUP("Basic", "");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "application_id"), "set_application_id", "get_application_id");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "release_version", PROPERTY_HINT_RANGE, "0,65535,1"), "set_release_version", "get_release_version");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "display_version"), "set_display_version", "get_display_version");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "demo"), "set_demo", "is_demo");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "legal_information", PROPERTY_HINT_FILE, "*.zip"), "set_legal_information", "get_legal_information");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "localization_metadata", PROPERTY_HINT_TYPE_STRING, itos(Variant::OBJECT) + "/" + itos(PROPERTY_HINT_RESOURCE_TYPE) + ":LocalizationMetadataNX"), "set_localization_metadata", "get_localization_metadata");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "rating_metadata", PROPERTY_HINT_TYPE_STRING, itos(Variant::OBJECT) + "/" + itos(PROPERTY_HINT_RESOURCE_TYPE) + ":RatingMetadataNX"), "set_rating_metadata", "get_rating_metadata");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "appropriate_age_for_china", PROPERTY_HINT_ENUM, ages_for_china_raw_string), "set_appropriate_age_for_china", "get_appropriate_age_for_china");
	ADD_GROUP("Advanced", "");
	ADD_SUBGROUP("Save Data", "save_data_");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "save_data_size"), "set_save_data_size", "get_save_data_size");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "save_data_journal_size"), "set_save_data_journal_size", "get_save_data_journal_size");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "data_loss_confirmation_required"), "set_data_loss_confirmation_required", "is_data_loss_confirmation_required");

	for (const String &age_for_china : String(ages_for_china_raw_string).split(",")) {
		supported_ages_for_china.insert(age_for_china.strip_edges());
	}
}

void PublishingMetadataNX::set_application_id(const String &p_application_id) {
	application_id = p_application_id;
}

String PublishingMetadataNX::get_application_id() const {
	return application_id;
}

void PublishingMetadataNX::set_release_version(int p_release_version) {
	release_version = p_release_version;
}

int PublishingMetadataNX::get_release_version() const {
	return release_version;
}

void PublishingMetadataNX::set_display_version(const String &p_display_version) {
	display_version = p_display_version;
}

String PublishingMetadataNX::get_display_version() const {
	return display_version;
}

void PublishingMetadataNX::set_demo(bool p_demo) {
	demo = p_demo;
}

bool PublishingMetadataNX::is_demo() const {
	return demo;
}

void PublishingMetadataNX::set_legal_information(const String &p_legal_information) {
	legal_information = p_legal_information;
}

String PublishingMetadataNX::get_legal_information() const {
	return legal_information;
}

void PublishingMetadataNX::set_localization_metadata(const Array &p_localization_metadata) {
	localization_metadata = p_localization_metadata;
}

Array PublishingMetadataNX::get_localization_metadata() const {
	return localization_metadata;
}

int PublishingMetadataNX::get_localization_count() const {
	return localization_metadata.size();
}

Ref<LocalizationMetadataNX> PublishingMetadataNX::get_localization(int p_index) const {
	ERR_FAIL_INDEX_V(p_index, localization_metadata.size(), nullptr);
	return localization_metadata[p_index];
}

void PublishingMetadataNX::set_rating_metadata(const Array &p_rating_metadata) {
	rating_metadata = p_rating_metadata;
}

Array PublishingMetadataNX::get_rating_metadata() const {
	return rating_metadata;
}

int PublishingMetadataNX::get_rating_count() const {
	return rating_metadata.size();
}

Ref<RatingMetadataNX> PublishingMetadataNX::get_rating(int p_index) const {
	ERR_FAIL_INDEX_V(p_index, rating_metadata.size(), nullptr);
	return rating_metadata[p_index];
}

bool PublishingMetadataNX::has_rating_metadata() const {
	return !rating_metadata.is_empty();
}

void PublishingMetadataNX::set_appropriate_age_for_china(const String &p_age_for_china) {
	appropriate_age_for_china = p_age_for_china;
}

String PublishingMetadataNX::get_appropriate_age_for_china() const {
	return appropriate_age_for_china;
}

bool PublishingMetadataNX::is_valid_age_for_china(const String &p_age_for_china) {
	return supported_ages_for_china.has(p_age_for_china);
}

void PublishingMetadataNX::set_save_data_size(const String &p_save_data_size) {
	save_data_size = p_save_data_size;
}

String PublishingMetadataNX::get_save_data_size() const {
	return save_data_size;
}

void PublishingMetadataNX::set_save_data_journal_size(const String &p_save_data_journal_size) {
	save_data_journal_size = p_save_data_journal_size;
}

String PublishingMetadataNX::get_save_data_journal_size() const {
	return save_data_journal_size;
}

void PublishingMetadataNX::set_data_loss_confirmation_required(bool p_data_loss_confirmation_required) {
	data_loss_confirmation_required = p_data_loss_confirmation_required;
}

bool PublishingMetadataNX::is_data_loss_confirmation_required() const {
	return data_loss_confirmation_required;
}
