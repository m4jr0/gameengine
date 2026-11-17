# Standalone assetc verification checklist

These changes were source-reviewed only. **Compilation and runtime verification are pending.**

## Build boundary

- Configure and build the `assetc` target independently on Linux and Windows.
- Confirm `assetc` links only `comet_core`, `comet_data`, and the declared third-party dependencies, never `comet_runtime`, `comet_render`, or `comet_engine`.
- Inspect the link map for unwanted engine symbols.
- Build without relying on precompiled headers to catch missing direct includes.

## CLI smoke tests

- `assetc --help`: prints usage and exits successfully without initializing the scheduler.
- `assetc --force`: rebuilds assets using default directories.
- `assetc --assets assets --resources resources`: uses supplied paths.
- `assetc --assets=assets --resources=resources --force`: same behavior with equals syntax.
- `assetc --unknown`, `assetc --assets`, `assetc --force=true`, and repeated options: fail with a nonzero exit code.
- Verify paths with spaces and non-ASCII characters on each supported OS.

## Resource round-trip tests

For each resource type (material, texture, shader module, shader, static model, skeletal model, skeleton, animation clip):

1. Construct a representative resource with nonempty data and nested arrays.
2. Pack with `CompressionMode::None`, unpack using independent scratch and payload allocators, and compare all fields and byte buffers.
3. Repeat with `CompressionMode::Lz4`.
4. Verify serialized bytes against a fixture produced before the codec extraction (where available).
5. Destroy scratch storage immediately after unpack; ensure decoded payload remains valid until its owning allocator is destroyed.
6. Check empty arrays, large arrays, and malformed/truncated files; do not assume malformed input is currently safe.

## Lifetime and concurrency

- Run asset export with multiple model materials and animations.
- Confirm nested model jobs finish before the importer, scene context, and resource buffers are destroyed.
- Confirm all output files are saved before metadata is updated.
- Run a second pass without `--force` and verify unchanged assets are skipped.
- Run with `--force` and verify assets are rebuilt.
- Use ASan/UBSan on Linux; use available debug allocator checks on Windows.
- Verify scheduler shutdown and allocation tracking report no outstanding work or leaks.

## Known limitations

- Export orchestration is currently synchronous at the asset level; exporters may still schedule their own jobs.
- Resource decoders use inherited offset/count handling; add bounded readers separately.
- The core CLI parser intentionally supports only named flags and string-valued options, not positional arguments or subcommands.

## Automated CLI smoke test

After building assetc, run `bash src/tool/assetc/test_cli.sh /path/to/assetc`.
This checks parsing without writing assets. Resource round-trip and lifetime tests
still require compiled fixtures and sanitizer runs.
