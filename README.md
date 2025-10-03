# ETG-Mobile
Enter The Gungeon Mobile Port With Mods

Updated 1.0.400 Oct 2 2025

Saves are stored at Android/data/com.xd.etg.taptap/files/SlotA_****.save -- You can replace with your PC saves


<img width="1663" height="925" alt="image" src="https://github.com/user-attachments/assets/e111b266-a9ee-43f8-b5ca-61680ef47bc7" />

## Usage
1. Download the APK in [releases](https://github.com/TheExploration/ETG-Mobile/releases/latest)
2. Make sure your system language is set to English.
3. Accept the agreement on the main menu.
4. Use mod menu or press "TAPTAP"


## Building
Clone the repository
```console
git clone https://github.com/TheExploration/ETG-Mobile.git
```
Build the ModMenu
```console
cd TemplateModMenu
gradlew assembleDebug
```
Add the mod menu to the game apk
```console
cd ..
<copy game apk to current dir>
python build.py base.apk
```

Then sign the apk and done

## Credits
- [Zygisk-Il2CppDumper](https://github.com/Perfare/Zygisk-Il2CppDumper)
- [frida-il2cpp-bridge](https://github.com/vfsfitvnm/frida-il2cpp-bridge)
- [ModTemplate](https://github.com/catlowlevel/ModTemplate)

## Special Thanks
 - [TrueMLGPro](https://github.com/TrueMLGPro)
