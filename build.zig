const std = @import("std");
// Although this function looks imperative, note that its job is to
// declaratively construct a build graph that will be executed by an external
// runner.
pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const exe = b.addExecutable(.{
        .name = "funnygame",
        .target = target,
        .optimize = optimize,
    });
    exe.linkLibC();
    exe.linkLibCpp();
    exe.addCSourceFiles(.{
        .files = &.{
            // common
            "common/common.c",
            "common/cvar.c",
            "common/cmd.c",

            // client
            "client/client.c",
            "client/render.c",
            "client/vma.cpp",
            "client/render/model.c",
        },
    });
    if (target.result.os.tag == .windows) {
        exe.linkSystemLibrary("ws2_32");
        exe.linkSystemLibrary("vulkan-1");
        exe.addCSourceFiles(.{
            .files = &.{
                // files
                "windows/main.c",
                "windows/window.c",
                //"windows/module.c",
            },
        });
    }
    if (target.result.os.tag == .linux) {
        exe.linkSystemLibrary("X11");
        exe.linkSystemLibrary("vulkan");
        exe.addCSourceFiles(.{
            .files = &.{
                // files
                "linux/main.c",
                "linux/window.c",
                "linux/module.c",
            },
        });
    }
    exe.addIncludePath(b.path("./"));
    exe.addIncludePath(b.path("./includes/vulkan/include/"));
    exe.addLibraryPath(b.path("./includes"));

    const _libbrv = b.dependency("libbrv", .{
        .target = target,
        .optimize = optimize,
    });
    const libbrv = _libbrv.artifact("brv");
    exe.linkLibrary(libbrv);

    const tools = b.addExecutable(.{
        .name = "tools",
        .target = target,
        .optimize = optimize,
    });
    tools.linkLibC();
    tools.addCSourceFiles(.{ .files = &.{ "tools/main.c", "tools/model.c" } });
    tools.linkLibrary(libbrv);
    b.exe_dir = "bin/";

    b.installArtifact(tools);
    b.installArtifact(exe);

    const run_cmd = b.addRunArtifact(exe);

    run_cmd.step.dependOn(b.getInstallStep());

    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);
}
