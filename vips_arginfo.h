/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: b0a895aa400527f647c2f9bd728c4e8a94cb0c9b */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_vips_image_new_from_file, 0, 1, MAY_BE_ARRAY|MAY_BE_LONG)
	ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_ARRAY, 1, "[]")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_vips_image_new_from_buffer, 0, 1, MAY_BE_ARRAY|MAY_BE_LONG)
	ZEND_ARG_TYPE_INFO(0, buffer, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, option_string, IS_STRING, 1, "\"\"")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_ARRAY, 1, "[]")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_vips_image_new_from_array, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, array, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, scale, IS_DOUBLE, 1, "1.0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, offset, IS_DOUBLE, 1, "0.0")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_vips_image_write_to_file, 0, 2, MAY_BE_ARRAY|MAY_BE_LONG)
	ZEND_ARG_INFO(0, image)
	ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_ARRAY, 1, "[]")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_vips_image_write_to_buffer, 0, 2, MAY_BE_ARRAY|MAY_BE_LONG)
	ZEND_ARG_INFO(0, image)
	ZEND_ARG_TYPE_INFO(0, suffix, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_ARRAY, 1, "[]")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_vips_image_copy_memory, 0, 1, MAY_BE_ARRAY|MAY_BE_LONG)
	ZEND_ARG_INFO(0, image)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_vips_image_new_from_memory, 0, 5, MAY_BE_ARRAY|MAY_BE_LONG)
	ZEND_ARG_TYPE_INFO(0, memory, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, bands, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, format, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_vips_image_write_to_memory, 0, 1, MAY_BE_STRING|MAY_BE_LONG)
	ZEND_ARG_INFO(0, image)
ZEND_END_ARG_INFO()

#define arginfo_vips_image_write_to_array arginfo_vips_image_copy_memory

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_vips_foreign_find_load, 0, 1, MAY_BE_STRING|MAY_BE_LONG)
	ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_vips_foreign_find_load_buffer, 0, 1, MAY_BE_STRING|MAY_BE_LONG)
	ZEND_ARG_TYPE_INFO(0, buffer, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_vips_interpolate_new, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_vips_call, 0, 2, MAY_BE_ARRAY|MAY_BE_LONG)
	ZEND_ARG_TYPE_INFO(0, operation_name, IS_STRING, 0)
	ZEND_ARG_INFO(0, instance)
	ZEND_ARG_VARIADIC_TYPE_INFO(0, args, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_vips_image_get, 0, 2, MAY_BE_ARRAY|MAY_BE_LONG)
	ZEND_ARG_INFO(0, image)
	ZEND_ARG_TYPE_INFO(0, field, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_vips_image_get_typeof, 0, 2, IS_LONG, 0)
	ZEND_ARG_INFO(0, image)
	ZEND_ARG_TYPE_INFO(0, field, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_vips_image_set, 0, 3, IS_LONG, 0)
	ZEND_ARG_INFO(0, image)
	ZEND_ARG_TYPE_INFO(0, field, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_vips_type_from_name, 0, 1, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_vips_image_set_type, 0, 4, IS_LONG, 0)
	ZEND_ARG_INFO(0, image)
	ZEND_ARG_TYPE_MASK(0, type, MAY_BE_STRING|MAY_BE_LONG, NULL)
	ZEND_ARG_TYPE_INFO(0, field, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
ZEND_END_ARG_INFO()

#define arginfo_vips_image_remove arginfo_vips_image_get_typeof

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_vips_error_buffer, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_vips_cache_set_max, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_vips_cache_set_max_mem arginfo_vips_cache_set_max

#define arginfo_vips_cache_set_max_files arginfo_vips_cache_set_max

#define arginfo_vips_concurrency_set arginfo_vips_cache_set_max

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_vips_cache_get_max, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_vips_cache_get_max_mem arginfo_vips_cache_get_max

#define arginfo_vips_cache_get_max_files arginfo_vips_cache_get_max

#define arginfo_vips_cache_get_size arginfo_vips_cache_get_max

#define arginfo_vips_concurrency_get arginfo_vips_cache_get_max

#define arginfo_vips_version arginfo_vips_error_buffer


ZEND_FUNCTION(vips_image_new_from_file);
ZEND_FUNCTION(vips_image_new_from_buffer);
ZEND_FUNCTION(vips_image_new_from_array);
ZEND_FUNCTION(vips_image_write_to_file);
ZEND_FUNCTION(vips_image_write_to_buffer);
ZEND_FUNCTION(vips_image_copy_memory);
ZEND_FUNCTION(vips_image_new_from_memory);
ZEND_FUNCTION(vips_image_write_to_memory);
ZEND_FUNCTION(vips_image_write_to_array);
ZEND_FUNCTION(vips_foreign_find_load);
ZEND_FUNCTION(vips_foreign_find_load_buffer);
ZEND_FUNCTION(vips_interpolate_new);
ZEND_FUNCTION(vips_call);
ZEND_FUNCTION(vips_image_get);
ZEND_FUNCTION(vips_image_get_typeof);
ZEND_FUNCTION(vips_image_set);
ZEND_FUNCTION(vips_type_from_name);
ZEND_FUNCTION(vips_image_set_type);
ZEND_FUNCTION(vips_image_remove);
ZEND_FUNCTION(vips_error_buffer);
ZEND_FUNCTION(vips_cache_set_max);
ZEND_FUNCTION(vips_cache_set_max_mem);
ZEND_FUNCTION(vips_cache_set_max_files);
ZEND_FUNCTION(vips_concurrency_set);
ZEND_FUNCTION(vips_cache_get_max);
ZEND_FUNCTION(vips_cache_get_max_mem);
ZEND_FUNCTION(vips_cache_get_max_files);
ZEND_FUNCTION(vips_cache_get_size);
ZEND_FUNCTION(vips_concurrency_get);
ZEND_FUNCTION(vips_version);


static const zend_function_entry ext_functions[] = {
	ZEND_FE(vips_image_new_from_file, arginfo_vips_image_new_from_file)
	ZEND_FE(vips_image_new_from_buffer, arginfo_vips_image_new_from_buffer)
	ZEND_FE(vips_image_new_from_array, arginfo_vips_image_new_from_array)
	ZEND_FE(vips_image_write_to_file, arginfo_vips_image_write_to_file)
	ZEND_FE(vips_image_write_to_buffer, arginfo_vips_image_write_to_buffer)
	ZEND_FE(vips_image_copy_memory, arginfo_vips_image_copy_memory)
	ZEND_FE(vips_image_new_from_memory, arginfo_vips_image_new_from_memory)
	ZEND_FE(vips_image_write_to_memory, arginfo_vips_image_write_to_memory)
	ZEND_FE(vips_image_write_to_array, arginfo_vips_image_write_to_array)
	ZEND_FE(vips_foreign_find_load, arginfo_vips_foreign_find_load)
	ZEND_FE(vips_foreign_find_load_buffer, arginfo_vips_foreign_find_load_buffer)
	ZEND_FE(vips_interpolate_new, arginfo_vips_interpolate_new)
	ZEND_FE(vips_call, arginfo_vips_call)
	ZEND_FE(vips_image_get, arginfo_vips_image_get)
	ZEND_FE(vips_image_get_typeof, arginfo_vips_image_get_typeof)
	ZEND_FE(vips_image_set, arginfo_vips_image_set)
	ZEND_FE(vips_type_from_name, arginfo_vips_type_from_name)
	ZEND_FE(vips_image_set_type, arginfo_vips_image_set_type)
	ZEND_FE(vips_image_remove, arginfo_vips_image_remove)
	ZEND_FE(vips_error_buffer, arginfo_vips_error_buffer)
	ZEND_FE(vips_cache_set_max, arginfo_vips_cache_set_max)
	ZEND_FE(vips_cache_set_max_mem, arginfo_vips_cache_set_max_mem)
	ZEND_FE(vips_cache_set_max_files, arginfo_vips_cache_set_max_files)
	ZEND_FE(vips_concurrency_set, arginfo_vips_concurrency_set)
	ZEND_FE(vips_cache_get_max, arginfo_vips_cache_get_max)
	ZEND_FE(vips_cache_get_max_mem, arginfo_vips_cache_get_max_mem)
	ZEND_FE(vips_cache_get_max_files, arginfo_vips_cache_get_max_files)
	ZEND_FE(vips_cache_get_size, arginfo_vips_cache_get_size)
	ZEND_FE(vips_concurrency_get, arginfo_vips_concurrency_get)
	ZEND_FE(vips_version, arginfo_vips_version)
	ZEND_FE_END
};
