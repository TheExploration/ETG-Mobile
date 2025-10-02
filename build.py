#! /usr/bin/python
import xml.etree.ElementTree as ET
import os
import re
from sys import argv
import shutil

ET.register_namespace("android", "http://schemas.android.com/apk/res/android")

mode = "debug"
do_clean_up = True


def get_main_activity_name(manifest_file):
    try:
        # Define the Android namespace
        android_ns = {"android": "http://schemas.android.com/apk/res/android"}

        # Parse the AndroidManifest.xml file
        tree = ET.parse(manifest_file)
        root = tree.getroot()

        # Find the main activity by inspecting the provided activity tag
        main_activity_name = root.find(
            ".//activity[@android:name]", namespaces=android_ns
        )
        if main_activity_name is not None:
            return main_activity_name.attrib[
                "{http://schemas.android.com/apk/res/android}name"
            ]

    except ET.ParseError as e:
        print(f"Error parsing AndroidManifest.xml: {str(e)}")
    except Exception as e:
        print(f"Error: {str(e)}")

    return None


def get_activity_path(target_path: str, activity_name: str):
    # Iterate over files in the directory
    for root, _, files in os.walk(target_path):
        for file in files:
            # Check if the file name matches the last part of the main activity name
            expected_path = os.path.join(*activity_name.split(".")) + ".smali"
            result = os.path.join(root, file)

            if result.endswith(expected_path):
                # Print the full path to the matching file
                return result
    return None


def update_manifest_file(manifest_file: str):
    # Update the AndroidManifest.xml file with the main activity name
    tree = ET.parse(manifest_file)
    root = tree.getroot()
    # first_permission = root.find(".//permission", namespaces=android_ns)
    # if first_permission is None:
    # print("first_permission is None")
    # return
    new_permission = ET.Element(
        "uses-permission", {"android:name": "android.permission.SYSTEM_ALERT_WINDOW"}
    )
    root.insert(0, new_permission)

    # Create a new <service> element with the specified attributes
    new_service = ET.Element(
        "service",
        {
            "android:name": "com.android.support.Launcher",
            "android:enabled": "true",
            "android:exported": "false",
            "android:stopWithTask": "true",
        },
    )

    application_tag = root.find(".//application")
    if application_tag is not None:
        application_tag.insert(len(application_tag), new_service)

    tree.write(manifest_file)


def update_activity_file(main_activity: str):
    with open(main_activity) as f:
        content = f.read()
        pattern = r"\.method.+onCreate.+\n+.+"
        matched = re.search(pattern, content)
        if matched:
            on_create = matched.group()
            new_content = f"{on_create}\n\t{'invoke-static {p0}, Lcom/android/support/Main;->Start(Landroid/content/Context;)V'}"
            content = content.replace(on_create, new_content)
            with open(main_activity, "w") as f:
                f.write(content)


def get_filename(apk: str):
    if apk.endswith(".apk"):
        return apk[:-4]  # Remove last 4 characters (.apk)
    return apk


def decompile_apk(apk: str):
    # get filename
    filename = get_filename(apk)

    if os.path.exists(filename):
        shutil.rmtree(filename)

    # run a command shell
    print(f"Decompiling {apk}")
    code = os.system(f"apktool d {apk}")
    if code != 0:
        print(f"apktool d {apk} failed")
        exit(1)
    print(f"Decompiling {apk} done")
    return filename


def recompile(apk: str):
    filename = get_filename(apk)
    print(f"Recompiling {filename}")
    code = os.system(f"apktool b {filename}")
    if code != 0:
        print(f"apktool b {filename} failed")
        exit(1)
    return os.path.join(filename, "dist", f"{filename}.apk")


def get_unified_filename(base_apk: str, split_apk: str):
    """Generate consistent naming for output files from both APKs"""
    base_name = get_filename(base_apk)
    split_name = get_filename(split_apk)
    # Use base APK name as primary, add split identifier if different
    if base_name == split_name:
        return base_name
    else:
        return f"{base_name}_{split_name}"


def validate_base_apk(apk_dir: str):
    """Ensure base APK has required structure"""
    manifest_file = os.path.join(apk_dir, "AndroidManifest.xml")
    if not os.path.exists(manifest_file):
        print(f"Error: AndroidManifest.xml not found in {apk_dir}")
        return False
    return True


def validate_split_apk(apk_dir: str, selected_arch: str):
    """Ensure split APK has lib directory structure"""
    lib_dir = os.path.join(apk_dir, "lib", selected_arch)
    if not os.path.exists(lib_dir):
        print(f"Error: lib/{selected_arch} directory not found in {apk_dir}")
        return False
    return True


def process_base_apk(base_apk: str, menu_dir: str):
    """Handle base APK: manifest, activity, smali files"""
    base_dir = decompile_apk(base_apk)
    print(f"Processing base APK: {base_dir}")
    
    # Validate base APK structure
    if not validate_base_apk(base_dir):
        exit(1)
    
    manifest_file = os.path.join(base_dir, "AndroidManifest.xml")
    
    print(f"Looking for main activity in {manifest_file}")
    main_activity = get_main_activity_name(manifest_file)
    if not main_activity:
        print("Main activity not found in AndroidManifest.xml")
        exit(1)
    print(f"Main activity found: {main_activity}")
    
    print(f"Updating AndroidManifest.xml")
    update_manifest_file(manifest_file)
    print("Done")
    
    smali_dirs = []
    for dir in sorted(os.listdir(base_dir)):
        if "smali" in dir:
            smali_dirs.append(dir)
    last_smali = smali_dirs[-1]
    
    print(f"Looking for activity file in {base_dir}")
    activity_file = get_activity_path(base_dir, main_activity)
    if activity_file is None:
        print("Activity file not found")
        exit(1)
    print(f"Activity file found: {activity_file}")
    
    print(f"Updating activity file")
    update_activity_file(activity_file)
    print("Done")
    
    # Copy smali files only (no lib copying for base APK)
    src = os.path.join(menu_dir, "smali")
    dst = os.path.join(base_dir, last_smali)
    print(f"Copying smali files {src} to {dst}")
    shutil.copytree(src, dst, dirs_exist_ok=True)
    
    return base_dir


def process_split_apk(split_apk: str, menu_dir: str, selected_arch: str):
    """Handle split APK: library files only"""
    split_dir = decompile_apk(split_apk)
    print(f"Processing split APK: {split_dir}")
    
    # Validate split APK structure
    if not validate_split_apk(split_dir, selected_arch):
        exit(1)
    
    # Copy lib files only
    src = os.path.join(menu_dir, "lib", selected_arch, "libMyLibName.so")
    dst = os.path.join(split_dir, "lib", selected_arch)
    print(f"Copying library {src} to {dst}")
    shutil.copy(src, dst)
    
    # Delete other arch inside lib beside selected_arch
    lib_dir = os.path.join(split_dir, "lib")
    if os.path.exists(lib_dir):
        other_arch = os.listdir(lib_dir)
        other_arch = [arch for arch in other_arch if arch != selected_arch]
        
        for arch in other_arch:
            arch_path = os.path.join(lib_dir, arch)
            print(f"Deleting {arch_path}")
            shutil.rmtree(arch_path)
    
    return split_dir


def main(argv: list[str]):
    # Check for both single APK (legacy) and split APK modes
    if len(argv) < 2:
        print("Usage: python3 build.py <base.apk> <split_config.apk> [--release] [--keep] [--arm]")
        print("   or: python3 build.py <single.apk> [--release] [--keep] [--arm] (legacy mode)")
        exit(1)

    arch = "arm64"
    selected_arch = "arm64-v8a"
    global mode
    global do_clean_up
    if "--release" in argv:
        argv.remove("--release")
        mode = "release"
        print("Release mode")
    if "--keep" in argv:
        argv.remove("--keep")
        do_clean_up = False
    if "--arm" in argv:
        argv.remove("--arm")
        arch = "arm"

    # Determine if we're in single APK (legacy) or split APK mode
    is_split_mode = len(argv) >= 3
    
    if is_split_mode:
        # Split APK mode
        base_apk = argv[1]
        split_apk = argv[2]
        print("Split APK mode detected")
        
        if not base_apk.endswith(".apk") or not split_apk.endswith(".apk"):
            print("Usage: python3 build.py <base.apk> <split_config.apk>")
            exit(1)
    else:
        # Legacy single APK mode
        apk = argv[1]
        print("Single APK mode (legacy)")
        
        if not apk.endswith(".apk"):
            print("Usage: python3 build.py <single.apk>")
            exit(1)

    if arch == "arm":
        selected_arch = "armeabi-v7a"
        print("Arm mode")
    elif arch == "arm64":
        print("Arm64 mode")

    mod_menu_dir = "ModMenu"

    for dirs in os.listdir():
        if dirs.endswith("ModMenu"):
            mod_menu_dir = dirs

    if mod_menu_dir == "ModMenu":
        print("ModMenu dir not found")
        exit(1)

    menu_apk = os.path.join(
        mod_menu_dir, "app", "build", "intermediates", "apk", mode, f"app-{mode}.apk"
    )
    menu_apk2 = os.path.join(
        mod_menu_dir, "app", "build", "outputs", "apk", mode, f"app-{mode}.apk"
    )

    # check if menu_apk exists
    if not os.path.exists(menu_apk):
        if not os.path.exists(menu_apk2):
            print("Menu apk not found")
            exit(1)
        else:
            menu_apk = menu_apk2

    if is_split_mode:
        # Split APK processing
        unified_name = get_unified_filename(base_apk, split_apk)
        
        # copy
        shutil.copy(menu_apk, f"menu_{unified_name}.apk")
        menu_dir = decompile_apk(f"menu_{unified_name}.apk")
        print(f"Decompiled to {menu_dir}")
        
        # Process both APKs separately
        base_dir = process_base_apk(base_apk, menu_dir)
        split_dir = process_split_apk(split_apk, menu_dir, selected_arch)
        
        # Recompile both APKs
        base_result = recompile(base_apk)
        split_result = recompile(split_apk)
        
        # Generate output files
        base_output = f"{unified_name}_base_menued-{mode}.apk"
        split_output = f"{unified_name}_split_menued-{mode}.apk"
        
        shutil.copy(base_result, base_output)
        shutil.copy(split_result, split_output)
        
        print(f"Finished {base_output}")
        print(f"Finished {split_output}")
        
        if do_clean_up:
            print("Cleaning up...")
            shutil.rmtree(base_dir)
            shutil.rmtree(split_dir)
            shutil.rmtree(menu_dir)
            os.remove(f"menu_{unified_name}.apk")
    else:
        # Legacy single APK processing
        # copy
        shutil.copy(menu_apk, f"menu_{get_filename(apk)}.apk")
        
        menu_dir = decompile_apk(f"menu_{get_filename(apk)}.apk")
        print(f"Decompiled to {menu_dir}")
        
        # =======================
        
        target_dir = decompile_apk(apk)
        print(f"Decompiled to {target_dir}")
        
        manifest_file = os.path.join(target_dir, "AndroidManifest.xml")
        
        print(f"Looking for main activity in {manifest_file}")
        main_activity = get_main_activity_name(manifest_file)
        if not main_activity:
            print("Main activity not found in AndroidManifest.xml")
            exit(1)
        print(f"Main activity found: {main_activity}")
        
        print(f"Updating AndroidManifest.xml")
        update_manifest_file(manifest_file)
        print("Done")
        
        smali_dirs = []
        for dir in sorted(os.listdir(target_dir)):
            if "smali" in dir:
                smali_dirs.append(dir)
        last_smali = smali_dirs[-1]
        
        print(f"Looking for activity file in {target_dir}")
        activity_file = get_activity_path(target_dir, main_activity)
        if activity_file is None:
            print("Activity file not found")
            exit(1)
        print(f"Activity file found: {activity_file}")
        
        print(f"Updating activity file")
        update_activity_file(activity_file)
        print("Done")
        
        # copy
        src = os.path.join(menu_dir, "smali")
        dst = os.path.join(target_dir, last_smali)
        print(f"Copying {src} to {dst}")
        shutil.copytree(src, dst, dirs_exist_ok=True)
        src = os.path.join(menu_dir, "lib", selected_arch, "libMyLibName.so")
        dst = os.path.join(target_dir, "lib", selected_arch)
        print(f"Copying {src} to {dst}")
        shutil.copy(src, dst)
        
        # delete other arch inside lib beside selected_arch
        other_arch = os.listdir(os.path.join(target_dir, "lib"))
        other_arch = [arch for arch in other_arch if arch != selected_arch]
        
        for arch in other_arch:
            print(f"Deleting {os.path.join(target_dir, 'lib', arch)}")
            shutil.rmtree(os.path.join(target_dir, "lib", arch))
        
        result = recompile(apk)
        shutil.copy(result, f"{target_dir}_menued-{mode}.apk")
        print(f"Finished {target_dir}_menued-{mode}.apk")
        if do_clean_up:
            print("Cleaning up...")
            shutil.rmtree(target_dir)
            shutil.rmtree(menu_dir)
            os.remove(f"menu_{get_filename(apk)}.apk")
    print("Done")


print("Starting...")

if __name__ == "__main__":
    main(argv)
else:
    print("Not main")
