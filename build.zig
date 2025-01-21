const std = @import("std");
const shadercompiler = @import("shaders/shadercompiler.zig");
const builtin = @import("builtin");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const os = target.result.os.tag;
    const optimize = b.standardOptimizeOption(.{});
    const wf = b.addWriteFiles();
    _ = wf;

    // libbrv
    const _libbrv = b.dependency("libbrv", .{
        .target = target,
        .optimize = optimize,
    });
    const libbrv = _libbrv.artifact("brv");
    libbrv.root_module.addCMacro("BRV_NO_DESERIALIZATION", "");

    if (os == .windows) {
        const libbrv_dll = libbrv.getEmittedBin();
        b.getInstallStep().dependOn(&b.addInstallFileWithDir(libbrv_dll, .{ .custom = "../bin" }, "brv.dll").step);
    }

    // game source code

    const server_opt = b.option(bool, "server", "Enable configuration for server mode, by disabling any non-terminal output");
    const server = server_opt orelse false;

    const exe = b.addExecutable(.{
        .name = "funnygame",
        .target = target,
        .optimize = optimize,
    });
    if (server) {}
    exe.linkLibC();
    exe.linkLibCpp();
    exe.addCSourceFiles(.{
        .files = &.{
            // common
            "common/common.c",
            "common/cvar.c",
            "common/cmd.c",
            "common/model.c",
        },
    });
    if (!server) {
        exe.addCSourceFiles(.{
            .files = &.{
                // client
                "client/client.c",
                "client/main.c",
                // rendering
                "client/render.c",
                "client/vma.cpp",
                "client/render/model.c",
            },
        });
        if (os == .windows) {
            exe.linkSystemLibrary("vulkan-1");
            exe.addCSourceFile(.{ .file = b.path("windows/window.c") });
        }
        if (os == .linux) {
            exe.linkSystemLibrary("X11");
            exe.linkSystemLibrary("vulkan");
            exe.addCSourceFile(.{ .file = b.path("linux/window.c") });
        }
    }
    if (server) {
        exe.addCSourceFiles(.{
            .files = &.{
                // server
                "server/main.c",
                "server/server.c",
            },
        });
    }
    if (os == .windows) {
        exe.linkSystemLibrary("ws2_32");
        exe.addCSourceFiles(.{
            .files = &.{
                // files
                "windows/module.c",
            },
        });
    }
    if (os == .linux) {
        exe.addCSourceFiles(.{
            .files = &.{
                // files
                "linux/module.c",
            },
        });
    }
    exe.addIncludePath(b.path("./"));
    exe.addIncludePath(b.path("./includes/vulkan/include/"));
    exe.addIncludePath(b.path("./includes/Vulkan-Utility-Libraries/include/"));
    exe.addIncludePath(b.path("./includes/"));
    exe.addIncludePath(b.path("./modules/kernel/"));
    exe.addLibraryPath(b.path("./includes"));
    exe.linkLibrary(libbrv);
    const exeartifact = b.addInstallArtifact(exe, .{});
    exeartifact.dest_dir = .{ .custom = "../bin" };
    b.getInstallStep().dependOn(&exeartifact.step);

    // kernel module
    const kernelmodule = b.addSharedLibrary(.{
        .name = "kernel",
        .target = target,
        .optimize = optimize,
    });
    kernelmodule.addCSourceFiles(.{
        .files = &.{
            //
            "modules/kernel/source/systemcall.c",
        },
    });
    if (os == .windows) kernelmodule.addCSourceFile(.{ .file = b.path("modules/windows.c") });
    kernelmodule.addIncludePath(b.path("./modules/kernel/include"));

    const kernelmoduleartifact = b.addInstallArtifact(kernelmodule, .{});
    kernelmoduleartifact.dest_dir = .{ .custom = "../bin/kernel/" };
    if (os == .windows) kernelmoduleartifact.implib_dir = kernelmoduleartifact.dest_dir;
    if (os == .windows) kernelmoduleartifact.pdb_dir = kernelmoduleartifact.dest_dir;
    b.getInstallStep().dependOn(&kernelmoduleartifact.step);

    // tools
    const tools = b.addExecutable(.{
        .name = "tools",
        .target = target,
        .optimize = optimize,
    });
    tools.linkLibC();
    tools.addCSourceFiles(.{ .files = &.{ "tools/main.c", "tools/model.c" } });
    tools.linkLibrary(libbrv);
    tools.addIncludePath(b.path("includes/libbrv/include/"));

    const toolsartifact = b.addInstallArtifact(tools, .{});
    toolsartifact.dest_dir = .{ .custom = "../bin" };
    b.getInstallStep().dependOn(&toolsartifact.step);

    shadercompiler.compile(b, "shaders/mesh.slang");
    shadercompiler.compile(b, "shaders/mesh_soft.slang");

    // run it
    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(b.getInstallStep());

    if (b.args) |args| {
        run_cmd.addArgs(args);
    }
    run_cmd.setCwd(b.path("./bin/"));

    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);
}
