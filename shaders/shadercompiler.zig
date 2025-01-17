const std = @import("std");
const builtin = @import("builtin");

pub fn compile(b: *std.Build, path: []const u8) void {
    var slangc: []const u8 = undefined;
    if (builtin.os.tag == .windows) {
        slangc = "slangc.exe";
    } else {
        slangc = "slangc";
    }
    const shadercompiler_cmd = b.addSystemCommand(&.{
        slangc,
        "-target",
        "spirv",
        "-o",
    });
    const path2: []const u8 = replacefilename(path) catch |errormsg| {
        std.debug.print("Error occurred: {}\n", .{errormsg});
        return;
    };

    const output = shadercompiler_cmd.addOutputFileArg(path);
    b.getInstallStep().dependOn(&b.addInstallFileWithDir(output, .{ .custom = "../bin" }, path2).step);

    shadercompiler_cmd.addFileArg(b.path(path));
}

fn replacefilename(input: []const u8) ![]const u8 {
    var builder = std.ArrayList(u8).init(std.heap.page_allocator);

    var i: usize = 0;
    const slang = ".slang";
    const spv = ".spv";

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
