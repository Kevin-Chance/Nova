import os
import shutil
import re

root_dir = os.path.abspath(os.getcwd())
exclude_dirs = {'Inject', '.git', 'build', 'out', '.vs', '.vscode', 'dependencies', 'thirdparty'}

def is_excluded(path):
    parts = os.path.relpath(path, root_dir).split(os.sep)
    return any(p in exclude_dirs for p in parts)

# 1. Structural changes
src_ecos = os.path.join(root_dir, 'src', 'ecos')
if os.path.exists(src_ecos):
    for item in os.listdir(src_ecos):
        shutil.move(os.path.join(src_ecos, item), os.path.join(root_dir, 'src', item))
    os.rmdir(src_ecos)

include_ecos = os.path.join(root_dir, 'include', 'ecos')
include_nova = os.path.join(root_dir, 'include', 'nova')
if os.path.exists(include_ecos):
    os.rename(include_ecos, include_nova)

# 2. Text replacements
replacements = [
    (r'nova_ecos', 'nova'),
    (r'NOVA_ECOS', 'NOVA'),
    (r'Nova_Ecos', 'Nova'),
    (r'ecos_nova', 'nova'), # just in case inside texts
    (r'ecos', 'nova'),
    (r'Ecos', 'Nova'),
    (r'ECOS', 'NOVA')
]

for dirpath, dirnames, filenames in os.walk(root_dir, topdown=False):
    if is_excluded(dirpath):
        continue

    for filename in filenames:
        if filename == 'refactor.py':
            continue
        file_path = os.path.join(dirpath, filename)
        
        # Content replacement
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                content = f.read()
            
            new_content = content
            for old, new in replacements:
                new_content = re.sub(old, new, new_content)
            
            # Special fix for src/CMakeLists.txt sources
            if file_path.replace('\\', '/').endswith('src/CMakeLists.txt'):
                new_content = re.sub(r'"nova/([^"]*\.cpp)"', r'"\1"', new_content)
                new_content = re.sub(r'"nova/([^"]*\.cpp\.in)"', r'"\1"', new_content)
                new_content = new_content.replace('${CMAKE_INSTALL_INCLUDEDIR}/nova', '${CMAKE_INSTALL_INCLUDEDIR}/nova') # Just checking it's fine

            if new_content != content:
                with open(file_path, 'w', encoding='utf-8') as f:
                    f.write(new_content)
        except Exception as e:
            pass # skip binary or unreadable files

        # File rename
        new_filename = filename
        for old, new in replacements:
            new_filename = re.sub(old, new, new_filename)
        
        if new_filename != filename:
            os.rename(file_path, os.path.join(dirpath, new_filename))

    # Directory rename
    for dirname in dirnames:
        if is_excluded(os.path.join(dirpath, dirname)):
            continue
        new_dirname = dirname
        for old, new in replacements:
            new_dirname = re.sub(old, new, new_dirname)
        if new_dirname != dirname:
            os.rename(os.path.join(dirpath, dirname), os.path.join(dirpath, new_dirname))

print("Refactoring complete.")
