import os
import glob
import shutil

workspace = "c:/Users/24512/Desktop/ecos_nova"

# 1. Rename directories
dirs_to_rename = [
    (os.path.join(workspace, "src", "components", "listeners"), os.path.join(workspace, "src", "components", "recorder")),
    (os.path.join(workspace, "include", "nova", "components", "listeners"), os.path.join(workspace, "include", "nova", "components", "recorder"))
]

for src_dir, dst_dir in dirs_to_rename:
    if os.path.exists(src_dir):
        os.rename(src_dir, dst_dir)
        print(f"Renamed {src_dir} to {dst_dir}")

# 2. Delete unwanted directories
dirs_to_delete = [
    os.path.join(workspace, "novapy.egg-info"),
    os.path.join(workspace, "out"),
    os.path.join(workspace, ".vs"),
    os.path.join(workspace, ".pytest_cache")
]

for d in dirs_to_delete:
    if os.path.exists(d):
        shutil.rmtree(d)
        print(f"Deleted {d}")

# 3. Replace occurrences in code
extensions = ['*.cpp', '*.hpp', '*.h', 'CMakeLists.txt']

for ext in extensions:
    for filepath in glob.glob(os.path.join(workspace, '**', ext), recursive=True):
        if 'external' in filepath or 'dependencies' in filepath or 'thirdparty' in filepath or '.git' in filepath or 'Inject' in filepath:
            continue
        
        try:
            with open(filepath, 'r', encoding='utf-8') as f:
                content = f.read()
                
            if 'components/listeners' in content or 'COMPONENTS_LISTENERS' in content:
                new_content = content.replace('components/listeners', 'components/recorder')
                new_content = new_content.replace('COMPONENTS_LISTENERS', 'COMPONENTS_RECORDER')
                
                with open(filepath, 'w', encoding='utf-8') as f:
                    f.write(new_content)
                print(f"Updated {filepath}")
        except Exception as e:
            pass

# 4. Update .gitignore
gitignore_path = os.path.join(workspace, ".gitignore")
if os.path.exists(gitignore_path):
    with open(gitignore_path, 'r', encoding='utf-8') as f:
        gitignore_content = f.read()
        
    additions = []
    if '.vs/' not in gitignore_content:
        additions.append(".vs/")
    if '.vscode/' not in gitignore_content:
        additions.append(".vscode/")
    if '.pytest_cache/' not in gitignore_content:
        additions.append(".pytest_cache/")
        
    if additions:
        with open(gitignore_path, 'a', encoding='utf-8') as f:
            f.write("\n" + "\n".join(additions) + "\n")
        print("Updated .gitignore")
