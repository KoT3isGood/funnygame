const std = @import("std");
const builtin = @import("builtin");

pub fn compile(b: *std.Build, tool: *std.Build.Step.Compile, step: *std.Build.Step, path: []const u8) void {
    const run_cmd = b.addRunArtifact(tool);
    run_cmd.step.dependOn(b.getInstallStep());
    run_cmd.addArg("-o");
    const path2: []const u8 = replacefilename(path) catch |errormsg| {
        std.debug.print("Error occurred: {}\n", .{errormsg});
        return;
    };
    const file = run_cmd.addOutputFileArg(path2);
    run_cmd.addArg("-m");
    run_cmd.addFileArg(b.path(path));
    step.dependOn(&b.addInstallFileWithDir(file, .{ .custom = "../bin" }, path2).step);
}

fn replacefilename(input: []const u8) ![]const u8 {
    var builder = std.ArrayList(u8).init(std.heap.page_allocator);

    var i: usize = 0;
    const slang = ".obj";
    const spv = ".bmf";

    while (i < input.len) {
        if (i + slang.len <= input.len and std.mem.eql(u8, input[i .. i + slang.len], slang)) {
            // If found, append ".spv"
            try builder.appendSlice(spv);
            i += slang.len;
        } else {
            // Else, append the current character
            try builder.append(input[i]);

            i += 1;
        }
    }

    return builder.toOwnedSlice();
}
