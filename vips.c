/**
 * C layer of the php-vips binding.
 */

/* Uncomment for some logging.
#define VIPS_DEBUG
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "SAPI.h"
#include "php_vips.h"

#include <vips/vips.h>
#include <vips/debug.h>
#include <vips/vector.h>

/* If you declare any globals in php_vips.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(vips)
*/

/* True global resources - no need for thread safety here */
static int le_gobject;

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("vips.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_vips_globals, vips_globals)
    STD_PHP_INI_ENTRY("vips.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_vips_globals, vips_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ proto static int vips_php_call_array(const char *operation_name, zval *instance, const char *option_string, int argc, zval *argv, zval *return_value)
   Call any vips operation. */

/* Track stuff during a call to a vips operation in one of these.
 */
typedef struct _VipsPhpCall {
	/* Parameters.
	 */
	const char *operation_name;
	zval *instance;
	const char *option_string;
	int argc;
	zval *argv;

	/* The operation we are calling.
	 */
	VipsOperation *operation;

	/* The num of args this operation needs from php. This does not include the
	 * @instance zval.
	 */
	int args_required;

	/* If we've already used the instance zval.
	 */
	gboolean used_instance;

	/* Extra php array of optional args.
	 */
	zval *options;

	/* The first image arg ... the thing we expand constants to match.
	 */
	VipsImage *match_image;

} VipsPhpCall;

static void
vips_php_call_free(VipsPhpCall *call)
{
	VIPS_DEBUG_MSG("vips_php_call_free:\n");

	VIPS_UNREF(call->operation);
	g_free(call);
}

static VipsPhpCall *
vips_php_call_new(const char *operation_name, zval *instance, 
	const char *option_string, int argc, zval *argv)
{
	VipsPhpCall *call;

	VIPS_DEBUG_MSG("vips_php_call_new: %s\n", operation_name );
	VIPS_DEBUG_MSG("    option_string = \"%s\", argc = %d\n", 
			option_string, argc);

	call = g_new0( VipsPhpCall, 1 );
	call->operation_name = operation_name;
	call->instance = instance;
	call->option_string = option_string;
	call->argc = argc;
	call->argv = argv;

	if (!(call->operation = vips_operation_new(operation_name))) {
		vips_php_call_free(call);
		return NULL;
	}

	return call;
}

/* Get a non-reference zval. In php7, zvalues can be references to other zvals
 * ... chase down a chain of refs to get a real zval.
 * the ref pointer chain.
 */
static inline zval *
zval_get_nonref(zval *zvalue)
{
	while (Z_TYPE_P(zvalue) == IS_REFERENCE)
		zvalue = Z_REFVAL_P(zvalue);

	return zvalue;
}

/* First pass over our arguments: find the first image arg and note as
 * match_image.
 */
static void
vips_php_analyze_arg(VipsPhpCall *call, zval *zvalue)
{
	zvalue = zval_get_nonref(zvalue); 

	if (Z_TYPE_P(zvalue) == IS_ARRAY) {
		const int n = zend_hash_num_elements(Z_ARRVAL_P(zvalue));

		int i;

		for (i = 0; i < n; i++) { 
			zval *item = zend_hash_index_find(Z_ARRVAL_P(zvalue), i);

			if (item) {
				vips_php_analyze_arg(call, item);
			}
		}
	}
	else if (Z_TYPE_P(zvalue) == IS_RESOURCE) {
		VipsImage *image;

		if( (image = (VipsImage *)zend_fetch_resource(Z_RES_P(zvalue), 
			"GObject", le_gobject)) != NULL) {
			if (!call->match_image) {
				call->match_image = image;
			}
		}
	}
}

static int
vips_php_blob_free(void *buf, void *area)
{
	g_free(buf);

	return 0;
}

/* Expand a constant (eg. 12, "12" or [1, 2, 3]) into an image using 
 * @match_image as a guide.
 */
static VipsImage *
expand_constant(VipsImage *match_image, zval *constant)
{
	VipsImage *result;
	VipsImage *x;

	if (vips_black(&result, 1, 1, NULL)) {
		return NULL;
	}

	constant = zval_get_nonref(constant); 

	if (Z_TYPE_P(constant) == IS_ARRAY) {
		const int n = zend_hash_num_elements(Z_ARRVAL_P(constant));

		double *ones;
		double *offsets;
		int i;

		ones = VIPS_ARRAY(result, n, double);
		offsets = VIPS_ARRAY(result, n, double);

		for (i = 0; i < n; i++) {
			zval *ele;

			ones[i] = 1.0;

			if ((ele = zend_hash_index_find(Z_ARRVAL_P(constant), i)) != NULL) {
				offsets[i] = zval_get_double(ele);
			}
		}

		if (vips_linear(result, &x, ones, offsets, n, NULL)) {
			return NULL;
		}
		g_object_unref(result);
		result = x;
	}
	else {
		if (vips_linear1(result, &x, 1.0, zval_get_double(constant), NULL)) {
			return NULL;
		}
		g_object_unref(result);
		result = x;
	}

	if (vips_cast(result, &x, match_image->BandFmt, NULL)) {
		return NULL;
	}
	g_object_unref(result);
	result = x;

	if (vips_embed(result, &x, 0, 0, match_image->Xsize, match_image->Ysize, 
		"extend", VIPS_EXTEND_COPY,
		NULL)) {
		return NULL;
	}
	g_object_unref(result);
	result = x;

	result->Type = match_image->Type;
	result->Xres = match_image->Xres;
	result->Yres = match_image->Yres;
	result->Xoffset = match_image->Xoffset;
	result->Yoffset = match_image->Yoffset;

	return result;
}

/* Is a zval a rectangular 2D array.
 */
static gboolean
is_2D(zval *array)
{
	int height;
	zval *row;
	int width;
	int y;

	array = zval_get_nonref(array); 

	if (Z_TYPE_P(array) != IS_ARRAY) {
		return FALSE;
	}

	height = zend_hash_num_elements(Z_ARRVAL_P(array));
	if ((row = zend_hash_index_find(Z_ARRVAL_P(array), 0)) == NULL ||
		!(row = zval_get_nonref(row)) || 
		Z_TYPE_P(row) != IS_ARRAY) { 
		return FALSE;
	}
	width = zend_hash_num_elements(Z_ARRVAL_P(row));

	for (y = 1; y < height; y++) {
		if ((row = zend_hash_index_find(Z_ARRVAL_P(array), y)) == NULL ||
			!(row = zval_get_nonref(row)) ||
			Z_TYPE_P(row) != IS_ARRAY ||
			zend_hash_num_elements(Z_ARRVAL_P(row)) != width) {
			return FALSE;
		}
	}

	return TRUE;
}

/* Make a vips matrix image from a 2D zval. @array must have passed is_2D()
 * before calling this.
 */
static VipsImage *
matrix_from_zval(zval *array)
{
	int width;
	int height;
	zval *row;
	VipsImage *mat;
	int x, y;

	array = zval_get_nonref(array); 

	height = zend_hash_num_elements(Z_ARRVAL_P(array));
	row = zend_hash_index_find(Z_ARRVAL_P(array), 0);
	row = zval_get_nonref(row); 
	g_assert(Z_TYPE_P(row) == IS_ARRAY);
	width = zend_hash_num_elements(Z_ARRVAL_P(row));
	mat = vips_image_new_matrix(width, height);

	for (y = 0; y < height; y++) {
		row = zend_hash_index_find(Z_ARRVAL_P(array), y);
		row = zval_get_nonref(row); 
		g_assert(Z_TYPE_P(row) == IS_ARRAY);
		g_assert(zend_hash_num_elements(Z_ARRVAL_P(row)) == width);

		for (x = 0; x < width; x++) {
			zval *ele;

			ele = zend_hash_index_find(Z_ARRVAL_P(row), x);
			*VIPS_MATRIX(mat, x, y) = zval_get_double(ele);
		}
	}

	return mat;
}

/* Turn a zval into an image. An image stays an image, a 2D array of numbers 
 * becomes a matrix image, a 1D array or a simple constant is expanded to 
 * match @match_image.
 */
static VipsImage *
imageize(VipsImage *match_image, zval *zvalue)
{
	VipsImage *image;

	zvalue = zval_get_nonref(zvalue); 

	if (Z_TYPE_P(zvalue) == IS_RESOURCE &&
		(image = (VipsImage *) 
			zend_fetch_resource(Z_RES_P(zvalue), "GObject", le_gobject))) { 
		return image;
	}
	else if (is_2D(zvalue)) {
		return matrix_from_zval(zvalue);
	}
	else if (match_image) {
		return expand_constant(match_image, zvalue);
	}
	else {
		php_error_docref(NULL, E_WARNING, "not a VipsImage");
		return NULL;
	}
}

static int
zval_to_array_image(VipsImage *match_image, zval *zvalue, GValue *gvalue)
{
	VipsImage **arr;
	VipsImage *image;
	int n;
	int i;

	zvalue = zval_get_nonref(zvalue); 

	if (Z_TYPE_P(zvalue) == IS_ARRAY) {
		n = zend_hash_num_elements(Z_ARRVAL_P(zvalue));
	}
	else {
		n = 1;
	}

	vips_value_set_array_image(gvalue, n);
	arr = vips_value_get_array_image(gvalue, NULL);

	if (Z_TYPE_P(zvalue) == IS_ARRAY) {
		for (i = 0; i < n; i++) {
			zval *ele;

			ele = zend_hash_index_find(Z_ARRVAL_P(zvalue), i);
			if (!ele) {
				php_error_docref(NULL, E_WARNING, "element missing from array");
				return -1;
			}

			if (!(image = imageize(match_image, ele))) {
				return -1;
			}

			arr[i] = image;
			g_object_ref(image);
		}
	}
	else {
		if (!(image = imageize(match_image, zvalue))) {
			return -1;
		}

		arr[0] = image;
		g_object_ref(image);
	}

	return 0;
}

/* Set a gvalue from a php value. 
 *
 * You must set the type of the gvalue before calling this to hint what kind 
 * of gvalue to make. For example if type is an enum, a zval string will be 
 * used to look up the enum nick.
 *
 * If non-NULL, @match_image is used to turn constants into images. 
 */
static int
vips_php_zval_to_gval(VipsImage *match_image, zval *zvalue, GValue *gvalue)
{
	GType type = G_VALUE_TYPE(gvalue);

	/* The fundamental type ... eg. G_TYPE_ENUM for a VIPS_TYPE_KERNEL, or
	 * G_TYPE_OBJECT for VIPS_TYPE_IMAGE().
	 */
	GType fundamental = G_TYPE_FUNDAMENTAL(type);

	VipsImage *image;
	zend_string *zstr;
	int enum_value;

	switch (fundamental) {
		case G_TYPE_STRING:
			/* These are GStrings, vips refstrings are handled by boxed, see 
			 * below.
			 */
			zstr = zval_get_string(zvalue);
			g_value_set_string(gvalue, ZSTR_VAL(zstr));
			zend_string_release(zstr); 
			break;

		case G_TYPE_OBJECT:
			if (!(image = imageize(match_image, zvalue))) {
				return -1;
			}
			g_value_set_object(gvalue, image);
			break;

		case G_TYPE_INT:
			g_value_set_int(gvalue, zval_get_long(zvalue));
			break;

		case G_TYPE_UINT64:
			g_value_set_uint64(gvalue, zval_get_long(zvalue));
			break;

		case G_TYPE_BOOLEAN:
			g_value_set_boolean(gvalue, zval_get_long(zvalue));
			break;

		case G_TYPE_ENUM:
			zvalue = zval_get_nonref(zvalue); 

			if (Z_TYPE_P(zvalue) == IS_LONG) {
				enum_value = Z_LVAL_P(zvalue);
			}
			else if (Z_TYPE_P(zvalue) == IS_DOUBLE) {
				enum_value = Z_DVAL_P(zvalue);
			}
			else {
				zstr = zval_get_string(zvalue);
				enum_value = vips_enum_from_nick("enum", type, ZSTR_VAL(zstr));
				if (enum_value < 0) {
					zend_string_release(zstr); 
					return -1;
				}
				zend_string_release(zstr); 
			}
			g_value_set_enum(gvalue, enum_value);
			break;

		case G_TYPE_FLAGS:
			g_value_set_flags(gvalue, zval_get_long(zvalue));
			break;

		case G_TYPE_DOUBLE:
			g_value_set_double(gvalue, zval_get_double(zvalue));
			break;

		case G_TYPE_BOXED:
			if (type == VIPS_TYPE_REF_STRING) {
				zstr = zval_get_string(zvalue);
				vips_value_set_ref_string(gvalue, ZSTR_VAL(zstr));
				zend_string_release(zstr); 
			}
			else if (type == VIPS_TYPE_BLOB) {
				void *buf;

				zvalue = zval_get_nonref(zvalue); 

				zstr = zval_get_string(zvalue);
				buf = g_malloc(ZSTR_LEN(zstr));
				memcpy(buf, ZSTR_VAL(zstr), ZSTR_LEN(zstr));
				zend_string_release(zstr); 

				vips_value_set_blob(gvalue, 
					vips_php_blob_free, buf, Z_STRLEN_P(zvalue));
			}
			else if (type == VIPS_TYPE_ARRAY_INT) {
				int *arr;
				int n;
				int i;

				zvalue = zval_get_nonref(zvalue); 

				if (Z_TYPE_P(zvalue) == IS_ARRAY) {
					n = zend_hash_num_elements(Z_ARRVAL_P(zvalue));
				}
				else {
					n = 1;
				}

				vips_value_set_array_int(gvalue, NULL, n);
				arr = vips_value_get_array_int(gvalue, NULL);

				if (Z_TYPE_P(zvalue) == IS_ARRAY) {
					for (i = 0; i < n; i++) {
						zval *ele;

						ele = zend_hash_index_find(Z_ARRVAL_P(zvalue), i);
						if (ele) { 
							arr[i] = zval_get_long(ele);
						}
					}
				}
				else {
					arr[0] = zval_get_long(zvalue);
				}
			}
			else if (type == VIPS_TYPE_ARRAY_DOUBLE) {
				double *arr;
				int n;
				int i;

				zvalue = zval_get_nonref(zvalue); 

				if (Z_TYPE_P(zvalue) == IS_ARRAY) {
					n = zend_hash_num_elements(Z_ARRVAL_P(zvalue));
				}
				else {
					n = 1;
				}

				vips_value_set_array_double(gvalue, NULL, n);
				arr = vips_value_get_array_double(gvalue, NULL);

				if (Z_TYPE_P(zvalue) == IS_ARRAY) {
					for (i = 0; i < n; i++) {
						zval *ele;

						ele = zend_hash_index_find(Z_ARRVAL_P(zvalue), i);
						if (ele) { 
							arr[i] = zval_get_double(ele);
						}
					}
				}
				else {
					arr[0] = zval_get_double(zvalue);
				}
			}
			else if (type == VIPS_TYPE_ARRAY_IMAGE) {
				if (zval_to_array_image(match_image, zvalue, gvalue)) {
					return -1;
				}
			}
			else {
				g_warning( "%s: unimplemented boxed type %s", 
					G_STRLOC, g_type_name(type) );
			}
			break;

		default:
			g_warning( "%s: unimplemented GType %s", 
				G_STRLOC, g_type_name(fundamental) );
			break;
	}

	return 0;
}

static int
vips_php_set_value(VipsPhpCall *call, 
	GParamSpec *pspec, VipsArgumentFlags flags, zval *zvalue)
{
	const char *name = g_param_spec_get_name(pspec);
	GType pspec_type = G_PARAM_SPEC_VALUE_TYPE(pspec);
	GValue gvalue = { 0 };

	g_value_init(&gvalue, pspec_type);
	if (vips_php_zval_to_gval(call->match_image, zvalue, &gvalue)) {
		g_value_unset(&gvalue);
		return -1;
	}

	/* If we are setting a MODIFY VipsArgument with an image, we need to take a
	 * copy.
	 */
	if (g_type_is_a(pspec_type, VIPS_TYPE_IMAGE) &&
		(flags & VIPS_ARGUMENT_MODIFY)) {
		VipsImage *image;
		VipsImage *memory;

		VIPS_DEBUG_MSG("vips_php_set_value: copying image\n");

		image = (VipsImage *) g_value_get_object(&gvalue);
		memory = vips_image_new_memory();
		if (vips_image_write(image, memory)) {
			g_object_unref(memory);
			g_value_unset(&gvalue);
			return -1;
		}
		g_value_unset(&gvalue);
		g_value_init(&gvalue, pspec_type);
		g_value_set_object(&gvalue, memory);
	}

#ifdef VIPS_DEBUG
{
	char *str_value;

	str_value = g_strdup_value_contents(&gvalue);
	VIPS_DEBUG_MSG("    %s.%s = %s\n", call->operation_name, name, str_value);
	g_free(str_value);
}
#endif/*VIPS_DEBUG*/

	g_object_set_property(G_OBJECT(call->operation), name, &gvalue);
	g_value_unset(&gvalue);

	return 0;
}

static void *
vips_php_set_required_input(VipsObject *object, 
	GParamSpec *pspec, VipsArgumentClass *argument_class, 
	VipsArgumentInstance *argument_instance, 
	void *a, void *b)
{
	VipsPhpCall *call = (VipsPhpCall *) a;

	if ((argument_class->flags & VIPS_ARGUMENT_REQUIRED) &&
		(argument_class->flags & VIPS_ARGUMENT_CONSTRUCT) &&
		(argument_class->flags & VIPS_ARGUMENT_INPUT) &&
		!(argument_class->flags & VIPS_ARGUMENT_DEPRECATED) &&
		!argument_instance->assigned) {
		zval *arg;

		/* If this arg needs an image, we use instance, if we can.
		 */
		arg = NULL;
		if (G_PARAM_SPEC_VALUE_TYPE(pspec) == VIPS_TYPE_IMAGE &&
			call->instance &&
			!call->used_instance) {
			arg = call->instance;
			call->used_instance = TRUE;
		}
		else if (call->args_required < call->argc) {
			/* Pick the next zval off the supplied arg list.
			 */
			arg = &call->argv[call->args_required];
			call->args_required += 1;
		}
				
		if (arg &&
			vips_php_set_value(call, pspec, argument_class->flags, arg)) {
			return call;
		}
	}

	return NULL;
}

/* Set all optional arguments.
 */
static int
vips_php_set_optional_input(VipsPhpCall *call, zval *options)
{
	zend_string *key;
	zval *value;

	VIPS_DEBUG_MSG("vips_php_set_optional_input:\n");

	options = zval_get_nonref(options); 

	ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(options), key, value) {
		char *name;
		GParamSpec *pspec;
		VipsArgumentClass *argument_class;
		VipsArgumentInstance *argument_instance;

		if (key == NULL) {
			continue;
		}

		name = ZSTR_VAL(key);
		if (vips_object_get_argument(VIPS_OBJECT(call->operation), name,
			&pspec, &argument_class, &argument_instance)) {
			return -1;
		}

		if (!(argument_class->flags & VIPS_ARGUMENT_REQUIRED) &&
			(argument_class->flags & VIPS_ARGUMENT_INPUT) &&
			!(argument_class->flags & VIPS_ARGUMENT_DEPRECATED) &&
			vips_php_set_value(call, pspec, argument_class->flags, value)) {
			return -1;
		}
	} ZEND_HASH_FOREACH_END();

	return 0;
}

/* Set a php zval from a gvalue. 
 */
static int
vips_php_gval_to_zval(GValue *gvalue, zval *zvalue)
{
	GType type = G_VALUE_TYPE(gvalue);

	/* The fundamental type ... eg. G_TYPE_ENUM for a VIPS_TYPE_KERNEL, or
	 * G_TYPE_OBJECT for VIPS_TYPE_IMAGE().
	 */
	GType fundamental = G_TYPE_FUNDAMENTAL(type);

	const char *str;
	GObject *gobject;
	zend_resource *resource;
	int enum_value;

	switch (fundamental) {
		case G_TYPE_STRING:
			/* These are GStrings, vips refstrings are handled by boxed, see 
			 * below.
			 */
			str = g_value_get_string(gvalue);
			ZVAL_STRING(zvalue, str);
			break;

		case G_TYPE_OBJECT:
			gobject = g_value_get_object(gvalue);
			resource = zend_register_resource(gobject, le_gobject);
			ZVAL_RES(zvalue, resource);
			break;

		case G_TYPE_INT:
			ZVAL_LONG(zvalue, g_value_get_int(gvalue));
			break;

		case G_TYPE_UINT64:
			ZVAL_LONG(zvalue, g_value_get_uint64(gvalue));
			break;

		case G_TYPE_BOOLEAN:
			ZVAL_LONG(zvalue, g_value_get_boolean(gvalue));
			break;

		case G_TYPE_ENUM:
			enum_value = g_value_get_enum(gvalue);
			str = vips_enum_nick(type, enum_value);
			ZVAL_STRING(zvalue, str);
			break;

		case G_TYPE_FLAGS:
			ZVAL_LONG(zvalue, g_value_get_flags(gvalue));
			break;

		case G_TYPE_DOUBLE:
			ZVAL_DOUBLE(zvalue, g_value_get_double(gvalue));
			break;

		case G_TYPE_BOXED:
			if (type == VIPS_TYPE_REF_STRING ||
				type == VIPS_TYPE_BLOB) {
				const char *str;
				size_t str_len;

				str = vips_value_get_ref_string(gvalue, &str_len);
				ZVAL_STRINGL(zvalue, str, str_len);
			}
			else if (type == VIPS_TYPE_ARRAY_DOUBLE) {
				double *arr;
				int n;
				int i;

				arr = vips_value_get_array_double(gvalue, &n);
				array_init(zvalue);
				for (i = 0; i < n; i++) {
					add_next_index_double(zvalue, arr[i]);
				}
			}
			else if (type == VIPS_TYPE_ARRAY_INT) {
				int *arr;
				int n;
				int i;

				arr = vips_value_get_array_int(gvalue, &n);
				array_init(zvalue);
				for (i = 0; i < n; i++) {
					add_next_index_long(zvalue, arr[i]);
				}
			}
			else if (type == VIPS_TYPE_ARRAY_IMAGE) {
				VipsImage **arr;
				int n;
				int i;

				arr = vips_value_get_array_image(gvalue, &n);
				array_init(zvalue);
				for (i = 0; i < n; i++) {
					zval x;

					g_object_ref(arr[i]);
					resource = zend_register_resource(arr[i], le_gobject);
					ZVAL_RES(&x, resource);
					add_next_index_zval(zvalue, &x);
				}
			}
			else {
				g_warning( "%s: unimplemented boxed type %s", 
					G_STRLOC, g_type_name(type));
			}
			break;

		default:
			g_warning( "%s: unimplemented GType %s", 
				G_STRLOC, g_type_name(fundamental));
			break;
	}

	return 0;
}

static int
vips_php_get_value(VipsPhpCall *call, GParamSpec *pspec, zval *zvalue)
{
	const char *name = g_param_spec_get_name(pspec);
	GType pspec_type = G_PARAM_SPEC_VALUE_TYPE(pspec);
	GValue gvalue = { 0 }; 

	g_value_init(&gvalue, pspec_type);
	g_object_get_property(G_OBJECT(call->operation), name, &gvalue);
	if (vips_php_gval_to_zval(&gvalue, zvalue)) {
		g_value_unset(&gvalue);
		return -1;
	}

#ifdef VIPS_DEBUG
{
	char *str_value;

	str_value = g_strdup_value_contents(&gvalue);
	VIPS_DEBUG_MSG("    %s.%s = %s\n", call->operation_name, name, str_value);
	g_free(str_value);
}
#endif/*VIPS_DEBUG*/

	g_value_unset(&gvalue);

	return 0;
}

static void *
vips_php_get_required_output(VipsObject *object, 
	GParamSpec *pspec, VipsArgumentClass *argument_class, 
	VipsArgumentInstance *argument_instance, 
	void *a, void *b)
{
	VipsPhpCall *call = (VipsPhpCall *) a;
	zval *return_value = (zval *) b;

	/* We get output objects, and we get input objects that are tagged as
	 * MODIFY --- they are copied on set, see above.
	 */
	if ((argument_class->flags & VIPS_ARGUMENT_REQUIRED) &&
		(argument_class->flags & (VIPS_ARGUMENT_OUTPUT| VIPS_ARGUMENT_MODIFY)) &&
		!(argument_class->flags & VIPS_ARGUMENT_DEPRECATED)) { 
		const char *name = g_param_spec_get_name(pspec);
		zval zvalue;

		if (vips_php_get_value(call, pspec, &zvalue)) {
			return call;
		}
		add_assoc_zval(return_value, name, &zvalue);
	}

	return NULL;
}

static int
vips_php_get_optional_output(VipsPhpCall *call, zval *options, 
	zval *return_value)
{
	zend_string *key;
	zval *value;

	VIPS_DEBUG_MSG("vips_php_get_optional_output:\n");

	options = zval_get_nonref(options); 

	ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(options), key, value) {
		char *name;
		GParamSpec *pspec;
		VipsArgumentClass *argument_class;
		VipsArgumentInstance *argument_instance;

		if (key == NULL) {
			continue;
		}

		/* value should always be TRUE. 
		 */
		value = zval_get_nonref(value); 
		if (Z_TYPE_P(value) != IS_TRUE) {
			continue;
		}

		name = ZSTR_VAL(key);
		if (vips_object_get_argument(VIPS_OBJECT(call->operation), name,
			&pspec, &argument_class, &argument_instance)) {
			return -1;
		}

		if (!(argument_class->flags & VIPS_ARGUMENT_REQUIRED) &&
			(argument_class->flags & VIPS_ARGUMENT_OUTPUT) &&
			!(argument_class->flags & VIPS_ARGUMENT_DEPRECATED)) {
			zval zvalue;

			if (vips_php_get_value(call, pspec, &zvalue)) {
				return -1;
			}

			add_assoc_zval(return_value, name, &zvalue);
		}
	} ZEND_HASH_FOREACH_END();

	return 0;
}

/* Call any vips operation, with the arguments coming from an array of zval. 
 * argv can have an extra final arg, which is an associative array of 
 * optional arguments. 
 */
static int
vips_php_call_array(const char *operation_name, zval *instance, 
	const char *option_string, int argc, zval *argv, zval *return_value)
{
	VipsPhpCall *call;
	int i;

	VIPS_DEBUG_MSG("vips_php_call_array:\n");

	if (!(call = vips_php_call_new(operation_name, instance, option_string,
		argc, argv))) {
		return -1;
	}

	/* Some initial analysis of our args. Loop over them all, including the
	 * special 'instance' arg.
	 */
	VIPS_DEBUG_MSG("vips_php_call_array: analyzing input args ...\n");
	if (call->instance) {
		vips_php_analyze_arg(call, call->instance);
	}
	for (i = 0; i < argc; i++) {
		vips_php_analyze_arg(call, &call->argv[i]);
	}

	/* Set str options before vargs options, so the user can't
	 * override things we set deliberately.
	 */
	VIPS_DEBUG_MSG("vips_php_call_array: setting args from option_string ...\n");
	if (option_string &&
		vips_object_set_from_string(VIPS_OBJECT(call->operation), 
			option_string)) {
		vips_object_unref_outputs(VIPS_OBJECT(call->operation));
		vips_php_call_free(call);
		return -1;
	}

	/* Set all required input args from argv.
	 */
	VIPS_DEBUG_MSG("vips_php_call_array: setting required input args ...\n");
	if (vips_argument_map(VIPS_OBJECT(call->operation), 
		vips_php_set_required_input, call, NULL)) {
		vips_object_unref_outputs(VIPS_OBJECT(call->operation));
		vips_php_call_free(call);
		return -1;
	}

	/* args_required must match argc, or we allow one extra final arg for options.
	 */
	VIPS_DEBUG_MSG("vips_php_call_array: testing argc ...\n");
	if (call->argc == call->args_required + 1) {
		/* Make sure it really is an array.
		 */
		if (zend_parse_parameter(0, call->argc - 1, &call->argv[call->argc - 1],
			"a", &call->options) == FAILURE) {
			vips_object_unref_outputs(VIPS_OBJECT(call->operation));
			vips_php_call_free(call);
			return -1;
		}
	}
	else if (call->argc != call->args_required) {
		php_error_docref(NULL, E_WARNING, 
			"operation %s expects %d arguments, but you supplied %d",
			call->operation_name, call->args_required, call->argc);
		vips_object_unref_outputs(VIPS_OBJECT(call->operation));
		vips_php_call_free(call);
		return -1;
	}

	/* Set all optional arguments.
	 */
	VIPS_DEBUG_MSG("vips_php_call_array: setting optional input args ...\n");
	if (call->options &&
		vips_php_set_optional_input(call, call->options)) {
		vips_object_unref_outputs(VIPS_OBJECT(call->operation));
		vips_php_call_free(call);
		return -1;
	}

	/* Look up in cache and build.
	 */
	VIPS_DEBUG_MSG("vips_php_call_array: building ...\n");
	if (vips_cache_operation_buildp(&call->operation)) {
		VIPS_DEBUG_MSG("vips_php_call_array: call failed!\n");
		vips_object_unref_outputs(VIPS_OBJECT(call->operation));
		vips_php_call_free(call);
		return -1;
	}

	/* Walk args again, getting required output.
	 */
	VIPS_DEBUG_MSG("vips_php_call_array: getting required output ...\n");
	array_init(return_value);
	if (vips_argument_map(VIPS_OBJECT(call->operation), 
		vips_php_get_required_output, call, return_value)) {
		vips_object_unref_outputs(VIPS_OBJECT(call->operation));
		vips_php_call_free(call);
		return -1;
	}

	/* And optional output.
	 */
	VIPS_DEBUG_MSG("vips_php_call_array: getting optional output ...\n");
	if (call->options &&
		vips_php_get_optional_output(call, call->options, return_value)) {
		vips_object_unref_outputs(VIPS_OBJECT(call->operation));
		vips_php_call_free(call);
		return -1;
	}

	vips_php_call_free(call);

	VIPS_DEBUG_MSG("vips_php_call_array: success!\n");

	return 0;
}

/* }}} */

/* {{{ proto mixed vips_php_call(string operation_name, resource instance [, more])
   Call any vips operation */

PHP_FUNCTION(vips_call)
{
	int argc;
	zval *argv;
	char *operation_name;
	size_t operation_name_len;
	zval *instance;

	VIPS_DEBUG_MSG("vips_call:\n");

	argc = ZEND_NUM_ARGS();

	if (argc < 1) {
		WRONG_PARAM_COUNT;
	}

	argv = (zval *)emalloc(argc * sizeof(zval));

	if (zend_get_parameters_array_ex(argc, argv) == FAILURE) {
		efree(argv);
		WRONG_PARAM_COUNT;
	}

	if (zend_parse_parameter(0, 0, &argv[0], 
		"s", &operation_name, &operation_name_len) == FAILURE) {
		efree(argv);
		RETURN_LONG(-1);
	}

	if (zend_parse_parameter(0, 1, &argv[1], "r!", &instance) == FAILURE) {
		efree(argv);
		RETURN_LONG(-1);
	}

	if (vips_php_call_array(operation_name, instance, 
		"", argc - 2, argv + 2, return_value)) {
		efree(argv);
		RETURN_LONG(-1);
	}

	efree(argv);
}
/* }}} */

/* {{{ proto resource vips_image_new_from_file(string filename [, array options])
   Open an image from a filename */
PHP_FUNCTION(vips_image_new_from_file)
{
	char *name;
	size_t name_len;
	zval *options;
	char filename[VIPS_PATH_MAX];
	char option_string[VIPS_PATH_MAX];
	const char *operation_name;
	zval argv[2];
	int argc;

	VIPS_DEBUG_MSG("vips_image_new_from_file:\n");

	options = NULL;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "p|a", 
		&name, &name_len, &options) == FAILURE) {
		RETURN_LONG(-1);
	}
	VIPS_DEBUG_MSG("vips_image_new_from_file: name = %s\n", name);

	vips__filename_split8(name, filename, option_string);
	if (!(operation_name = vips_foreign_find_load(filename))) {
		RETURN_LONG(-1);
	}

	ZVAL_STRING(&argv[0], filename);
	argc = 1;
	if (options) {
		ZVAL_ARR(&argv[1], Z_ARR_P(options));
		argc += 1;
	}

	if (vips_php_call_array(operation_name, NULL, 
		option_string, argc, argv, return_value)) {
		zval_dtor(&argv[0]);
		RETURN_LONG(-1);
	}

	zval_dtor(&argv[0]);
}
/* }}} */

/* {{{ proto resource vips_image_new_from_buffer(string buffer [, string option_string, array options])
   Open an image from a string */
PHP_FUNCTION(vips_image_new_from_buffer)
{
	char *buffer;
	size_t buffer_len;
	char *option_string;
	size_t option_string_len;
	zval *options;
	const char *operation_name;
	zval argv[2];
	int argc;

	VIPS_DEBUG_MSG("vips_image_new_from_buffer:\n");

	option_string = NULL;
	options = NULL;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s|sa", 
		&buffer, &buffer_len, &option_string, &option_string_len, 
		&options) == FAILURE) {
		RETURN_LONG(-1);
	}

	if (!(operation_name = vips_foreign_find_load_buffer(buffer, buffer_len))) {
		RETURN_LONG(-1);
	}

	ZVAL_STRINGL(&argv[0], buffer, buffer_len);
	argc = 1;
	if (options) {
		ZVAL_ARR(&argv[1], Z_ARR_P(options));
		argc += 1;
	}

	if (vips_php_call_array(operation_name, NULL, 
		option_string, argc, argv, return_value)) {
		zval_dtor(&argv[0]);
		RETURN_LONG(-1);
	}

	zval_dtor(&argv[0]);
}
/* }}} */

/* {{{ proto resource vips_image_new_from_array(array coefficients [, double scale, double offset])
   Open an image from a string */
PHP_FUNCTION(vips_image_new_from_array)
{
	zval *array;
	double scale;
	double offset;
	int width;
	int height;
	VipsImage *mat;
	int x;
	zval *row;

	VIPS_DEBUG_MSG("vips_image_new_from_array:\n");

	scale = 1.0;
	offset = 0.0;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "a|dd", 
		&array, &scale, &offset) == FAILURE) {
		return;
	}

	height = zend_hash_num_elements(Z_ARRVAL_P(array));
	if ((row = zend_hash_index_find(Z_ARRVAL_P(array), 0)) == NULL) {
		php_error_docref(NULL, E_WARNING, "no element zero");
		return;
	}
	if (is_2D(array)) {
		mat =  matrix_from_zval(array);
	}
	else {
		/* 1D array.
		 */
		width = height;
		height = 1;

		mat = vips_image_new_matrix(width, height);

		for (x = 0; x < width; x++) {
			zval *ele;

			ele = zend_hash_index_find(Z_ARRVAL_P(array), x);
			if (ele) { 
				*VIPS_MATRIX(mat, x, 0) = zval_get_double(ele);
			}
		}
	}

	vips_image_set_double(mat, "scale", scale);
	vips_image_set_double(mat, "offset", offset);

	RETURN_RES(zend_register_resource(mat, le_gobject));
}
/* }}} */

/* {{{ proto resource vips_interpolate_new(string name])
   make a new interpolator */
PHP_FUNCTION(vips_interpolate_new)
{
	char *name;
	size_t name_len;
	VipsInterpolate *interp;

	VIPS_DEBUG_MSG("vips_interpolate_new:\n");

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "p", 
		&name, &name_len) == FAILURE) {
		return;
	}
	VIPS_DEBUG_MSG("vips_interpolate_new: name = %s\n", name);

	if (!(interp = vips_interpolate_new(name)))
		return;

	RETURN_RES(zend_register_resource(interp, le_gobject));
}
/* }}} */

/* {{{ proto long vips_image_write_to_file(resource image, string filename [, array options])
   Write an image to a filename */
PHP_FUNCTION(vips_image_write_to_file)
{
	zval *IM;
	char *filename;
	size_t filename_len;
	zval *options = NULL;
	VipsImage *image;
	char path_string[VIPS_PATH_MAX];
	char option_string[VIPS_PATH_MAX];
	const char *operation_name;
	zval argv[2];
	int argc;

	VIPS_DEBUG_MSG("vips_image_write_to_file:\n");

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rp|a", 
		&IM, &filename, &filename_len, &options) == FAILURE) {
		RETURN_LONG(-1);
	}

	if ((image = (VipsImage *)zend_fetch_resource(Z_RES_P(IM), 
		"GObject", le_gobject)) == NULL) {
		RETURN_LONG(-1);
	}

	VIPS_DEBUG_MSG("\t%p -> %s\n", image, filename);

	vips__filename_split8(filename, path_string, option_string);
	if (!(operation_name = vips_foreign_find_save(path_string))) {
		RETURN_LONG(-1);
	}

	ZVAL_STRINGL(&argv[0], filename, filename_len);
	argc = 1;
	if (options) {
		ZVAL_ARR(&argv[1], Z_ARR_P(options));
		argc += 1;
	}

	if (vips_php_call_array(operation_name, IM, 
		option_string, argc, argv, return_value)) {
		zval_dtor(&argv[0]);
		RETURN_LONG(-1);
	}

	zval_dtor(&argv[0]);
}
/* }}} */

/* {{{ proto string|long vips_image_write_to_buffer(resource image, string suffix [, array options])
   Write an image to a string */
PHP_FUNCTION(vips_image_write_to_buffer)
{
	zval *IM;
	zval *options = NULL;
	char *suffix;
	size_t suffix_len;
	VipsImage *image;
	char filename[VIPS_PATH_MAX];
	char option_string[VIPS_PATH_MAX];
	const char *operation_name;
	zval argv[1];
	int argc;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rs|a", 
		&IM, &suffix, &suffix_len, &options) == FAILURE) {
		RETURN_LONG(-1);
	}

	if ((image = (VipsImage *)zend_fetch_resource(Z_RES_P(IM), 
		"GObject", le_gobject)) == NULL) {
		RETURN_LONG(-1);
	}

	vips__filename_split8(suffix, filename, option_string);
	if (!(operation_name = vips_foreign_find_save_buffer(filename))) {
		RETURN_LONG(-1);
	}

	argc = 0;
	if (options) {
		ZVAL_ARR(&argv[0], Z_ARR_P(options));
		argc += 1;
	}

	if (vips_php_call_array(operation_name, IM, 
		option_string, argc, argv, return_value)) {
		RETURN_LONG(-1);
	}
}
/* }}} */

/* {{{ proto resource vips_image_copy_memory(resource image)
   Copy an image to a memory image */
PHP_FUNCTION(vips_image_copy_memory)
{
	zval *IM;
	VipsImage *image;
	VipsImage *new_image;
	zend_resource *resource;
	zval zvalue;

	VIPS_DEBUG_MSG("vips_image_copy_memory:\n");

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &IM) == FAILURE) {
		RETURN_LONG(-1);
	}

	if ((image = (VipsImage *)zend_fetch_resource(Z_RES_P(IM), 
		"GObject", le_gobject)) == NULL) {
		RETURN_LONG(-1);
	}

	if (!(new_image = vips_image_copy_memory(image))) {
		RETURN_LONG(-1);
	}

	/* Return as an array for all OK, -1 for error.
	 */
	array_init(return_value);
	resource = zend_register_resource(new_image, le_gobject);
	ZVAL_RES(&zvalue, resource);
	add_assoc_zval(return_value, "out", &zvalue);
}
/* }}} */

/* {{{ proto resource vips_image_write_to_memory(array data, integer width, integer height, integer bands, string format)
   Wrap an image around a memory array. */
PHP_FUNCTION(vips_image_new_from_memory)
{
	HashTable *ht;
	long width;
	long height;
	long bands;
	char *format;
	size_t format_len;
	int format_value;
	VipsBandFormat band_format;
	VipsImage *image;
	zend_resource *resource;
	zval zvalue;

	VIPS_DEBUG_MSG("vips_image_new_from_memory:\n");

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "hlllp",
		&ht, &width, &height, &bands, &format, &format_len) == FAILURE) {
		RETURN_LONG(-1);
	}

	if ((format_value = vips_enum_from_nick("enum", VIPS_TYPE_BAND_FORMAT, format)) < 0) {
		RETURN_LONG(-1);
	}
	band_format = format_value;

	const int size = zend_hash_num_elements(ht);
	int arr[size];
	int i;

	for (i = 0; i < size; i++) {
		zval *ele;

		if ((ele = zend_hash_index_find(ht, i)) != NULL) {
			arr[i] = zval_get_long(ele);
		}
	}

	if (!(image = vips_image_new_from_memory_copy(arr, size, width, height, bands,
		band_format))) {
		RETURN_LONG(-1);
	}

	/* Return as an array for all OK, -1 for error.
	 */
	array_init(return_value);
	resource = zend_register_resource(image, le_gobject);
	ZVAL_RES(&zvalue, resource);
	add_assoc_zval(return_value, "out", &zvalue);
}
/* }}} */

/* {{{ proto array vips_image_write_to_memory(resource image)
   Write an image to a memory array. */
PHP_FUNCTION(vips_image_write_to_memory)
{
	zval *IM;
	VipsImage *image;
	size_t arr_len;
	uint8_t *arr;

	VIPS_DEBUG_MSG("vips_image_write_to_memory:\n");

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &IM) == FAILURE) {
		RETURN_LONG(-1);
	}

	if ((image = (VipsImage *)zend_fetch_resource(Z_RES_P(IM),
		"GObject", le_gobject)) == NULL) {
		RETURN_LONG(-1);
	}

	if (!(arr = vips_image_write_to_memory(image, &arr_len))) {
		RETURN_LONG(-1);
	}

	array_init(return_value);

	int i;
	for (i = 0; i < arr_len; i++) {
		add_next_index_long(return_value, arr[i]);
	}
}
/* }}} */

/* {{{ proto string|long vips_foreign_find_load(string filename)
   Find a loader for a file */
PHP_FUNCTION(vips_foreign_find_load)
{
	char *filename;
	size_t filename_len;
	const char *operation_name;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", 
		&filename, &filename_len) == FAILURE) {
		RETURN_LONG(-1);
	}

	if (!(operation_name = vips_foreign_find_load(filename))) {
		RETURN_LONG(-1);
	}

	RETVAL_STRING(strdup(operation_name));
}
/* }}} */

/* {{{ proto string|long vips_foreign_find_load_buffer(string buffer)
   Find a loader for a buffer */
PHP_FUNCTION(vips_foreign_find_load_buffer)
{
	char *buffer;
	size_t buffer_len;
	const char *operation_name;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", 
		&buffer, &buffer_len) == FAILURE) {
		RETURN_LONG(-1);
	}

	if (!(operation_name = vips_foreign_find_load_buffer(buffer, buffer_len))) {
		RETURN_LONG(-1);
	}

	RETVAL_STRING(strdup(operation_name));
}
/* }}} */

/* {{{ proto array vips_image_get(resource image, string field)
   Fetch field from image */
PHP_FUNCTION(vips_image_get)
{
	zval *im;
	char *field_name;
	size_t field_name_len;
	VipsImage *image;
	GValue gvalue = { 0 };
	zval zvalue;
	GParamSpec *pspec;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rs", 
		&im, &field_name, &field_name_len) == FAILURE) {
		RETURN_LONG(-1);
	}

	if ((image = (VipsImage *)zend_fetch_resource(Z_RES_P(im), 
		"GObject", le_gobject)) == NULL) {
		RETURN_LONG(-1);
	}

	/* Ugly: older libvipses would return enums from the true header fields 
	 * (eg. ->interpretation) as ints, but we want to send a string back
	 * for things like this.
	 *
	 * Test if field_name exists as a regular glib property and if it does, use
	 * g_object_get(). Otherwise use vips_image_get(), since it can read extra
	 * image metadata.
	 */
	if ((pspec = g_object_class_find_property(G_OBJECT_GET_CLASS(image), 
		field_name))) {
		g_value_init(&gvalue, G_PARAM_SPEC_VALUE_TYPE(pspec));
		g_object_get_property(G_OBJECT(image), field_name, &gvalue);
	} 
	else if (vips_image_get(image, field_name, &gvalue)) {
		RETURN_LONG(-1);
	}

	if (vips_php_gval_to_zval(&gvalue, &zvalue)) {
		g_value_unset(&gvalue);
		RETURN_LONG(-1);
	}
	g_value_unset(&gvalue);

	/* Return as an array for all OK, -1 for error.
	 */
	array_init(return_value);
	add_assoc_zval(return_value, "out", &zvalue);
}
/* }}} */

/* {{{ proto long vips_image_get_typeof(resource image, string field)
   Fetch type of field from image */
PHP_FUNCTION(vips_image_get_typeof)
{
	zval *im;
	char *field_name;
	size_t field_name_len;
	VipsImage *image;
	GType type;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rs", 
		&im, &field_name, &field_name_len) == FAILURE) {
		RETURN_LONG(-1);
	}

	if ((image = (VipsImage *)zend_fetch_resource(Z_RES_P(im), 
		"GObject", le_gobject)) == NULL) {
		RETURN_LONG(-1);
	}

	type = vips_image_get_typeof(image, field_name); 
	
	RETURN_LONG(type);
}
/* }}} */

/* {{{ proto long vips_image_set(resource image, string field, mixed value)
   Set field on image */
PHP_FUNCTION(vips_image_set)
{
	zval *im;
	char *field_name;
	size_t field_name_len;
	zval *zvalue;
	VipsImage *image;
	GType type;
	GValue gvalue = { 0 };

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rsz", 
		&im, &field_name, &field_name_len, &zvalue) == FAILURE) {
		RETURN_LONG(-1);
	}

	if ((image = (VipsImage *)zend_fetch_resource(Z_RES_P(im), 
		"GObject", le_gobject)) == NULL) {
		RETURN_LONG(-1);
	}

	type = vips_image_get_typeof(image, field_name); 

	/* If the type is not set, guess a default.
	 */
	if (type == 0) {
		zval *ele;

		type = 0;

		zvalue = zval_get_nonref(zvalue); 

		switch (Z_TYPE_P(zvalue)) {
			case IS_ARRAY:
				if ((ele = zend_hash_index_find(Z_ARRVAL_P(zvalue), 
					0)) != NULL) {
					ele = zval_get_nonref(ele); 

					switch (Z_TYPE_P(ele)) {
						case IS_RESOURCE:
							type = VIPS_TYPE_ARRAY_IMAGE;
							break;

						case IS_LONG:
							type = VIPS_TYPE_ARRAY_INT;
							break;

						case IS_DOUBLE:
							type = VIPS_TYPE_ARRAY_DOUBLE;
							break;

						default:
							break;
					}
				}
				break;

			case IS_RESOURCE:
				type = VIPS_TYPE_IMAGE;
				break;

			case IS_LONG:
				type = G_TYPE_INT;
				break;

			case IS_DOUBLE:
				type = G_TYPE_DOUBLE;
				break;

			case IS_STRING:
				type = VIPS_TYPE_REF_STRING;
				break;

			default:
				break;
		}
	}

	g_value_init(&gvalue, type);

	if (vips_php_zval_to_gval(NULL, zvalue, &gvalue)) {
		RETURN_LONG(-1);
	}

	vips_image_set(image, field_name, &gvalue);

	g_value_unset(&gvalue);

	RETURN_LONG(0);
}
/* }}} */

/* {{{ proto long vips_image_remove(resource image, string field)
   Remove field from image */
PHP_FUNCTION(vips_image_remove)
{
	zval *im;
	char *field_name;
	size_t field_name_len;
	VipsImage *image;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rs", 
		&im, &field_name, &field_name_len) == FAILURE) {
		RETURN_LONG(-1);
	}

	if ((image = (VipsImage *)zend_fetch_resource(Z_RES_P(im), 
		"GObject", le_gobject)) == NULL) {
		RETURN_LONG(-1);
	}

	if (!vips_image_remove(image, field_name)) {
		RETURN_LONG(-1);
	}
	
	RETURN_LONG(0);
}
/* }}} */

/* {{{ proto string vips_error_buffer()
   Fetch and clear the vips error buffer */
PHP_FUNCTION(vips_error_buffer)
{
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "") == FAILURE) {
		return;
	}

	RETVAL_STRING(strdup(vips_error_buffer()));
	vips_error_clear();
}
/* }}} */

/* {{{ proto void vips_cache_set_max(integer value)
   Set max number of operations to cache */
PHP_FUNCTION(vips_cache_set_max)
{
	long value;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &value) == FAILURE) {
		return;
	}

	vips_cache_set_max(value); 
}
/* }}} */

/* {{{ proto void vips_cache_set_max_mem(integer value)
   Set max memory to use for operation cache */
PHP_FUNCTION(vips_cache_set_max_mem)
{
	long value;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &value) == FAILURE) {
		return;
	}

	vips_cache_set_max_mem(value); 
}
/* }}} */

/* {{{ proto void vips_cache_set_max_files(integer value)
   Set max number of open files for operation cache */
PHP_FUNCTION(vips_cache_set_max_files)
{
	long value;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &value) == FAILURE) {
		return;
	}

	vips_cache_set_max_files(value); 
}
/* }}} */

/* {{{ proto void vips_concurrency_set(integer value)
   Set number of workers per threadpool */
PHP_FUNCTION(vips_concurrency_set)
{
	long value;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &value) == FAILURE) {
		return;
	}

	vips_concurrency_set(value); 
}
/* }}} */

/* {{{ php_vips_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_vips_init_globals(zend_vips_globals *vips_globals)
{
	vips_globals->global_value = 0;
	vips_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ php_free_vips_object
 *  */
static void php_free_gobject(zend_resource *rsrc)
{
	VIPS_DEBUG_MSG("php_free_gobject: %p\n", rsrc->ptr);

	g_object_unref((GObject *) rsrc->ptr);
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(vips)
{
	if (strcmp(sapi_module.name, "apache2handler") == 0) {
		/* "apachectl graceful" can cause us terrible problems. What happens:
		 *
		 * - the main apache process unloads this extension, vips.so
		 * - in turn, the C runtime will unload libvips.so, the vips library,
		 *   since vips.so is the only thing that references it
		 * - libvips.so in turn uses glib.so, but this is often not unloaded,
		 *   since other parts of apache can be using it (glib could also
		 *   possibly be preventing unload itself, I'm not sure)
		 * - the main apache process then reloads vips.so, which in turn will
		 *   reload libvips.so as it starts up
		 * - vips.so tries to init libvips.so
		 * - libvips.so tries to register its types (such as VipsImage) with
		 *   glib.so, but finds the types from the previous init still there
		 * - everything breaks
		 *
		 * A simple fix that will always work is just to lock libvips in 
		 * memory and prevent unload. We intentionally leak refs to the shared
		 * library. 
		 *
		 * We include the binary API version number that this extension needs. 
		 * We can't just load .so, that's only installed with libvips-dev, 
		 * which may not be present at runtime.
		 */
#ifdef VIPS_SONAME
		if (!dlopen(VIPS_SONAME, RTLD_LAZY | RTLD_NODELETE)) 
#else /*!VIPS_SONAME*/
		if (!dlopen("libvips.so.42", RTLD_LAZY | RTLD_NODELETE)) 
#endif /*VIPS_SONAME*/
		{
			sapi_module.sapi_error(E_WARNING, "php-vips-ext: unable to lock "
				"libvips -- graceful may be unreliable");
		}
	}

	/* If you have INI entries, uncomment these lines
	REGISTER_INI_ENTRIES();
	*/

	/* We're supposed to use the filename of something we think is in
	 * $VIPSHOME/bin, but we don't have that. Use a nonsense name and
	 * vips_init() will fall back to other techniques for finding data
	 * files.
	 */
	if (VIPS_INIT("banana"))
		return FAILURE;

	le_gobject = zend_register_list_destructors_ex(php_free_gobject, 
		NULL, "GObject", module_number);

#ifdef VIPS_DEBUG
	printf( "php-vips-ext init\n" );
	printf( "enabling vips leak testing ...\n" );
	vips_leak_set( TRUE ); 
#endif /*VIPS_DEBUG*/

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(vips)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/

#ifdef VIPS_DEBUG
	printf( "php-vips-ext shutdown\n" );
#endif /*VIPS_DEBUG*/

	/* We must not call vips_shutdown() since we've locked libvips in memory
	 * and will need to reuse it if we restart via graceful.
	 */

	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(vips)
{
#if defined(COMPILE_DL_VIPS) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(vips)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(vips)
{
	char digits[256];

	php_info_print_table_start();
	php_info_print_table_header(2, "vips property", "value");

	vips_snprintf(digits, 256, "%d.%d.%d", VIPS_MAJOR_VERSION, VIPS_MINOR_VERSION, VIPS_MICRO_VERSION);
	php_info_print_table_row(2, "Vips headers version", digits);
	vips_snprintf(digits, 256, "%d.%d.%d", vips_version(0), vips_version(1), vips_version(2));
	php_info_print_table_row(2, "Vips library version", digits);
	vips_snprintf(digits, 256, "%d.%d.%d", vips_version(3), vips_version(4), vips_version(5));
	php_info_print_table_row(2, "Vips ABI version", digits);

	vips_snprintf(digits, 256, "%d", vips_version(0));
	php_info_print_table_row(2, "Major version", digits); 
	vips_snprintf(digits, 256, "%d", vips_version(1));
	php_info_print_table_row(2, "Minor version", digits); 
	vips_snprintf(digits, 256, "%d", vips_version(2));
	php_info_print_table_row(2, "Micro version", digits); 

	php_info_print_table_row(2, "SIMD support with liborc", 
		vips_vector_isenabled() ? "yes" : "no" ); 

	php_info_print_table_row(2, "JPEG support", 
		vips_type_find("VipsOperation", "jpegload") ? "yes" : "no" ); 
	php_info_print_table_row(2, "PNG support", 
		vips_type_find("VipsOperation", "pngload") ? "yes" : "no" ); 
	php_info_print_table_row(2, "TIFF support", 
		vips_type_find("VipsOperation", "tiffload") ? "yes" : "no" ); 
	php_info_print_table_row(2, "GIF support", 
		vips_type_find("VipsOperation", "gifload") ? "yes" : "no" ); 
	php_info_print_table_row(2, "OpenEXR support", 
		vips_type_find("VipsOperation", "openexrload") ? "yes" : "no" ); 
	php_info_print_table_row(2, "load OpenSlide", 
		vips_type_find("VipsOperation", "openslideload") ? "yes" : "no" ); 
	php_info_print_table_row(2, "load Matlab", 
		vips_type_find("VipsOperation", "matload") ? "yes" : "no" ); 
	php_info_print_table_row(2, "load PDF", 
		vips_type_find("VipsOperation", "pdfload") ? "yes" : "no" ); 
	php_info_print_table_row(2, "load SVG", 
		vips_type_find("VipsOperation", "svgload") ? "yes" : "no" ); 
	php_info_print_table_row(2, "FITS support", 
		vips_type_find("VipsOperation", "fitsload") ? "yes" : "no" ); 
	php_info_print_table_row(2, "WebP support", 
		vips_type_find("VipsOperation", "webpload") ? "yes" : "no" ); 

	php_info_print_table_row(2, "load with libMagick", 
		vips_type_find("VipsOperation", "magickload") ? "yes" : "no" ); 

	php_info_print_table_row(2, "Text rendering support", 
		vips_type_find("VipsOperation", "text") ? "yes" : "no" ); 

	php_info_print_table_row(2, "ICC profile support with lcms", 
		vips_icc_present() ? "yes" : "no" ); 

	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */

ZEND_BEGIN_ARG_INFO(arginfo_vips_image_new_from_file, 0)
	ZEND_ARG_INFO(0, filename)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_vips_image_new_from_buffer, 0)
	ZEND_ARG_INFO(0, buffer)
	ZEND_ARG_INFO(0, option_string)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_vips_image_new_from_array, 0)
	ZEND_ARG_INFO(0, array)
	ZEND_ARG_INFO(0, scale)
	ZEND_ARG_INFO(0, offset)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_vips_image_write_to_file, 0)
	ZEND_ARG_INFO(0, image)
	ZEND_ARG_INFO(0, filename)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_vips_image_write_to_buffer, 0)
	ZEND_ARG_INFO(0, image)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_vips_image_copy_memory, 0)
	ZEND_ARG_INFO(0, image)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_vips_image_new_from_memory, 0)
	ZEND_ARG_INFO(0, array)
	ZEND_ARG_INFO(0, width)
	ZEND_ARG_INFO(0, height)
	ZEND_ARG_INFO(0, bands)
	ZEND_ARG_INFO(0, format)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_vips_image_write_to_memory, 0)
	ZEND_ARG_INFO(0, image)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_vips_foreign_find_load, 0)
	ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_vips_interpolate_new, 0)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_vips_foreign_find_load_buffer, 0)
	ZEND_ARG_INFO(0, buffer)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_vips_call, 0)
	ZEND_ARG_INFO(0, operation_name)
	ZEND_ARG_INFO(0, instance)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_vips_image_get, 0)
	ZEND_ARG_INFO(0, image)
	ZEND_ARG_INFO(0, field)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_vips_image_get_typeof, 0)
	ZEND_ARG_INFO(0, image)
	ZEND_ARG_INFO(0, field)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_vips_image_set, 0)
	ZEND_ARG_INFO(0, image)
	ZEND_ARG_INFO(0, field)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_vips_image_remove, 0)
	ZEND_ARG_INFO(0, image)
	ZEND_ARG_INFO(0, field)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_vips_error_buffer, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_vips_cache_set_max, 0)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_vips_cache_set_max_mem, 0)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_vips_cache_set_max_files, 0)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_vips_concurrency_set, 0)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

/* {{{ vips_functions[]
 *
 * Every user visible function must have an entry in vips_functions[].
 */
const zend_function_entry vips_functions[] = {
	PHP_FE(vips_image_new_from_file, arginfo_vips_image_new_from_file)
	PHP_FE(vips_image_new_from_buffer, arginfo_vips_image_new_from_buffer)
	PHP_FE(vips_image_new_from_array, arginfo_vips_image_new_from_array)
	PHP_FE(vips_image_write_to_file, arginfo_vips_image_write_to_file)
	PHP_FE(vips_image_write_to_buffer, arginfo_vips_image_write_to_buffer)
	PHP_FE(vips_image_copy_memory, arginfo_vips_image_copy_memory)
	PHP_FE(vips_image_new_from_memory, arginfo_vips_image_new_from_memory)
	PHP_FE(vips_image_write_to_memory, arginfo_vips_image_write_to_memory)
	PHP_FE(vips_foreign_find_load, arginfo_vips_foreign_find_load)
	PHP_FE(vips_foreign_find_load_buffer, arginfo_vips_foreign_find_load_buffer)
	PHP_FE(vips_interpolate_new, arginfo_vips_interpolate_new)

	PHP_FE(vips_call, arginfo_vips_call)
	PHP_FE(vips_image_get, arginfo_vips_image_get)
	PHP_FE(vips_image_get_typeof, arginfo_vips_image_get_typeof)
	PHP_FE(vips_image_set, arginfo_vips_image_set)
	PHP_FE(vips_image_remove, arginfo_vips_image_remove)
	PHP_FE(vips_error_buffer, arginfo_vips_error_buffer)
	PHP_FE(vips_cache_set_max, arginfo_vips_cache_set_max)
	PHP_FE(vips_cache_set_max_mem, arginfo_vips_cache_set_max_mem)
	PHP_FE(vips_cache_set_max_files, arginfo_vips_cache_set_max_files)
	PHP_FE(vips_concurrency_set, arginfo_vips_concurrency_set)

	PHP_FE_END	/* Must be the last line in vips_functions[] */
};
/* }}} */

/* {{{ vips_module_entry
 */
zend_module_entry vips_module_entry = {
	STANDARD_MODULE_HEADER,
	"vips",
	vips_functions,
	PHP_MINIT(vips),
	PHP_MSHUTDOWN(vips),
	PHP_RINIT(vips),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(vips),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(vips),
	PHP_VIPS_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_VIPS
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(vips)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
