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
    exe.linkSystemLibrary("X11");
    exe.addCSourceFiles(.{
        .files = &.{
            // files
            "linux/main.c",
            "linux/window.c",

            // common
            "common/common.c",
            "common/cvar.c",
            "common/cmd.c",

            // client
            "client/client.c",
        },
    });
    exe.addIncludePath(b.path("./"));

    const tools = b.addExecutable(.{
        .name = "tools",
        .target = target,
        .optimize = optimize,
    });
    tools.linkLibC();
    tools.addCSourceFiles(.{ .files = &.{ "tools/main.c", "tools/objtofmf.c" } });
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
