const std = @import("std");
const shadercompiler = @import("shaders/shadercompiler.zig");

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
    if (os == .windows) {
    }

    // game source code
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
    exe.addIncludePath(b.path("./modules/kernel/"));
    exe.addLibraryPath(b.path("./includes"));
    exe.linkLibrary(libbrv);
    const exeartifact = b.addInstallArtifact(exe,.{});
    exeartifact.dest_dir=.{.custom = "../bin"};
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
    kernelmodule.addIncludePath(b.path("./modules/kernel/include"));

    const kernelmoduleartifact = b.addInstallArtifact(kernelmodule, .{});
    kernelmoduleartifact.dest_dir = .{ .custom = "../bin/kernel/" };
    //kernelmoduleartifact.implib_dir = .{ .custom = "../bin/kernel/" };
    //kernelmoduleartifact.pdb_dir = .{ .custom = "../bin/kernel/" };
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


    const toolsartifact = b.addInstallArtifact(tools,.{});
    toolsartifact.dest_dir=.{.custom = "../bin"};
    b.getInstallStep().dependOn(&toolsartifact.step);

    shadercompiler.compile(b,"shaders/mesh.slang");

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


