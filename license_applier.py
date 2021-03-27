import os


def apply_header(filename, header):
    if not filename.endswith(".c") and not filename.endswith(".cpp") and not filename.endswith(".h"): return
    print("Applying header to", filename)
    with open(filename, "r+") as read_stream:
        first_line = read_stream.readline().strip()
        if first_line == "/**":
            print("Header may have been applied to", filename)
            return
        read_stream.seek(0)
        file_content = read_stream.read()
        temp_file = "/tmp/" + str(int.from_bytes(os.urandom(8), "big"))
        with open(temp_file, "w") as write_stream:
            write_stream.write(file_content) # write temporarily
    
    with open(filename, "w") as write_stream:
        write_stream.write(header)
        write_stream.write(file_content)
    
    print("Applied license header to", filename)
    os.remove(temp_file)


def list_files(directory="."):
    ls = []
    for filename in os.listdir(directory):
        if (filename == ".git"): continue
        absolute_path = os.path.join(directory, filename)
        if (os.path.isfile(absolute_path)):
            ls += [absolute_path]
        else:
            ls += list_files(absolute_path)
    return ls


git_output = list_files()

with open("LICENSE_HEADER.txt", "r") as read_stream:
    license_header = read_stream.read()

[apply_header(line.strip(), license_header) for line in git_output]
