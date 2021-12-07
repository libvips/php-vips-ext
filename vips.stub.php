<?php

/** @generate-class-entries */

/**
 * @return array<string, mixed>|int
 */
function vips_image_new_from_file(string $filename, ?array $options = []): array|int {}

/**
 * @return array<string, mixed>|int
 */
function vips_image_new_from_buffer(string $buffer, ?string $option_string = "", ?array $options = []): array|int {}

/**
 * @return resource
 */
function vips_image_new_from_array(array $array, ?float $scale = 1.0, ?float $offset = 0.0) {}

/**
 * @param resource $image
 * @return array<string, mixed>|int
 */
function vips_image_write_to_file($image, string $filename, ?array $options = []): array|int {}

/**
 * @param resource $image
 * @return array<string, mixed>|int
 */
function vips_image_write_to_buffer($image, string $suffix, ?array $options = []): array|int {}

/**
 * @param resource $image
 * @return array<string, resource>|int
 */
function vips_image_copy_memory($image): array|int {}

/**
 * @return array<string, resource>|int
 */
function vips_image_new_from_memory(string $memory, int $width, int $height, int $bands, string $format): array|int {}

/**
 * @param resource $image
 */
function vips_image_write_to_memory($image): string|int {}

/**
 * @param resource $image
 * @return array<int, mixed>|int
 */
function vips_image_write_to_array($image): array|int {}

function vips_foreign_find_load(string $filename): string|int {}

function vips_foreign_find_load_buffer(string $buffer): string|int {}

/**
 * @return resource
 */
function vips_interpolate_new(string $name) {}

/**
 * @param resource $instance
 * @return array<string, mixed>|int
 */
function vips_call(string $operation_name, $instance, mixed ...$args): array|int {}

/**
 * @param resource $image
 * @return array<string, mixed>|int
 */
function vips_image_get($image, string $field): array|int {}

/**
 * @param resource $image
 */
function vips_image_get_typeof($image, string $field): int {}

/**
 * @param resource $image
 */
function vips_image_set($image, string $field, mixed $value): int {}

function vips_type_from_name(string $name): int {}

/**
 * @param resource $image
 */
function vips_image_set_type($image, string|int $type, string $field, mixed $value): int {}

/**
 * @param resource $image
 */
function vips_image_remove($image, string $field): int {}

/**
 * @param resource $image
 * @return array<string, mixed>|int
 */
function vips_image_get_fields($image): array|int {}

function vips_error_buffer(): string {}

function vips_cache_set_max(int $value): void {}

function vips_cache_set_max_mem(int $value): void {}

function vips_cache_set_max_files(int $value): void {}

function vips_concurrency_set(int $value): void {}

function vips_cache_get_max(): int {}

function vips_cache_get_max_mem(): int {}

function vips_cache_get_max_files(): int {}

function vips_cache_get_size(): int {}

function vips_concurrency_get(): int {}

function vips_version(): string {}
