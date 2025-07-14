It might be the quickest cross-platform codesign alternative for iOS 12+, supporting macOS, Linux, Windows, and more features.
If this tool helps you, please don't forget to <font color=#FF0000 size=5>🌟**star**🌟</font> [ME](https://github.com/zhlynn).

```logs
/usr/local/bin/zsign -k '/home/appstore/certificates/p12/Certificate_dev_1234567890_abc123.p12' -m '/home/appstore/certificates/provision/Certificate_dev_profile_1234567890_def456.mobileprovision' -o '/home/appstore/output/signatures/xyz789AppSigned.ipa' -b '' -n '' '/home/appstore/builds/sample-app/Payload.ipa'  
>>> Unzip:      /home/appstore/builds/sample-app/Payload.ipa (28.45 MB) -> /tmp/zsign_folder_1234567890123456 ... 
>>> Unzip OK! (0.387s, 387234us)
>>> Found Assets.car - removing to force icon regeneration...
>>> Removed old Assets.car - new icons will be used
>>> Signing:    /tmp/zsign_folder_1234567890123456/Payload/SampleApp.app ...
>>> AppName:    Sample App
>>> BundleId:   dev.3zozz.sampleapp
>>> Version:    1.2
>>> TeamId:     A1B2C3D4E5
>>> SubjectCN:  iPhone Distribution: Dev 3zozz (A1B2C3D4E5)
>>> ReadCache:  NO
>>> AssetsCarHandled:   YES
>>> IconsChanged:       YES
>>> SignFile:   Frameworks/libswiftCoreMedia.dylib
>>> SignFile:   Frameworks/libswiftCoreData.dylib
>>> SignFile:   Frameworks/libswiftAVFoundation.dylib
>>> SignFile:   Frameworks/libswiftCoreLocation.dylib
>>> SignFile:   Frameworks/libswiftQuartzCore.dylib
>>> SignFile:   Frameworks/libswiftDarwin.dylib
>>> SignFile:   Frameworks/libswiftCoreGraphics.dylib
>>> SignFile:   Frameworks/libswiftMetal.dylib
>>> SignFile:   Frameworks/libswiftsimd.dylib
>>> SignFile:   Frameworks/libswiftCoreFoundation.dylib
>>> SignFile:   Frameworks/libswiftFoundation.dylib
>>> SignFile:   Frameworks/libswiftCoreImage.dylib
>>> SignFile:   Frameworks/libswiftCore.dylib
>>> SignFile:   Frameworks/libswiftCoreAudio.dylib
>>> SignFile:   Frameworks/libswiftNetwork.dylib
>>> SignFile:   Frameworks/libswiftUIKit.dylib
>>> SignFile:   Frameworks/libswift_Concurrency.dylib
>>> SignFile:   Frameworks/libswiftObjectiveC.dylib
>>> SignFile:   Frameworks/libswiftDispatch.dylib
>>> SignFile:   Frameworks/libswiftos.dylib
>>> SignFile:   Frameworks/libswiftPhotos.dylib
>>> SignFolder: Frameworks/image_processor.framework, (image_processor)
>>> CodeResources regenerated with current icons
>>> Can't find CodeSignature segment!
>>> Realloc CodeSignature space... 
>>> Success!
>>> SignFolder: Frameworks/NotificationService.framework, (NotificationService)
>>> CodeResources regenerated with current icons
>>> SignFolder: Frameworks/AnimationLib.framework, (AnimationLib)
>>> CodeResources regenerated with current icons
>>> Can't find CodeSignature segment!
>>> Realloc CodeSignature space... 
>>> Success!
>>> SignFolder: Frameworks/secure_storage.framework, (secure_storage)
>>> CodeResources regenerated with current icons
>>> Can't find CodeSignature segment!
>>> Realloc CodeSignature space... 
>>> Success!
>>> SignFolder: Frameworks/preferences_manager.framework, (preferences_manager)
>>> CodeResources regenerated with current icons
>>> Can't find CodeSignature segment!
>>> Realloc CodeSignature space... 
>>> Success!
>>> SignFolder: Frameworks/toast_notifications.framework, (toast_notifications)
>>> CodeResources regenerated with current icons
>>> Can't find CodeSignature segment!
>>> Realloc CodeSignature space... 
>>> Success!
>>> SignFolder: Frameworks/video_player.framework, (video_player)
>>> CodeResources regenerated with current icons
>>> Can't find CodeSignature segment!
>>> Realloc CodeSignature space... 
>>> Success!
>>> SignFolder: Frameworks/LocationServices.framework, (LocationServices)
>>> CodeResources regenerated with current icons
>>> SignFolder: Frameworks/PhotoLibrary.framework, (PhotoLibrary)
>>> CodeResources regenerated with current icons
>>> Can't find CodeSignature segment!
>>> Realloc CodeSignature space... 
>>> Success!
>>> SignFolder: Frameworks/InAppMessaging.framework, (InAppMessaging)
>>> CodeResources regenerated with current icons
>>> SignFolder: Frameworks/PushNotifications.framework, (PushNotifications)
>>> CodeResources regenerated with current icons
>>> SignFolder: Frameworks/url_handler.framework, (url_handler)
>>> CodeResources regenerated with current icons
>>> Can't find CodeSignature segment!
>>> Realloc CodeSignature space... 
>>> Success!
>>> SignFolder: Frameworks/CoreFramework.framework, (CoreFramework)
>>> CodeResources regenerated with current icons
>>> Can't find CodeSignature segment!
>>> Realloc CodeSignature space... 
>>> Success!
>>> SignFolder: Frameworks/ImageLoader.framework, (ImageLoader)
>>> CodeResources regenerated with current icons
>>> Can't find CodeSignature segment!
>>> Realloc CodeSignature space... 
>>> Success!
>>> SignFolder: Frameworks/App.framework, (App)
>>> CodeResources regenerated with current icons
>>> No enough CodeSignature space (now: 81920, need: 110000).
>>> Realloc CodeSignature space... 
>>> Success!
>>> SignFolder: Frameworks/connectivity_manager.framework, (connectivity_manager)
>>> CodeResources regenerated with current icons
>>> Can't find CodeSignature segment!
>>> Realloc CodeSignature space... 
>>> Success!
>>> SignFolder: Frameworks/deep_links.framework, (deep_links)
>>> CodeResources regenerated with current icons
>>> Can't find CodeSignature segment!
>>> Realloc CodeSignature space... 
>>> Success!
>>> SignFolder: Frameworks/CoreServices.framework, (CoreServices)
>>> CodeResources regenerated with current icons
>>> SignFolder: Frameworks/ImageCropper.framework, (ImageCropper)
>>> CodeResources regenerated with current icons
>>> Can't find CodeSignature segment!
>>> Realloc CodeSignature space... 
>>> Success!
>>> SignFolder: Frameworks/file_manager.framework, (file_manager)
>>> CodeResources regenerated with current icons
>>> Can't find CodeSignature segment!
>>> Realloc CodeSignature space... 
>>> Success!
>>> SignFolder: Frameworks/UserManager.framework, (UserManager)
>>> CodeResources regenerated with current icons
>>> SignFolder: Frameworks/LiveActivities.framework, (LiveActivities)
>>> CodeResources regenerated with current icons
>>> SignFolder: Frameworks/database_helper.framework, (database_helper)
>>> CodeResources regenerated with current icons
>>> Can't find CodeSignature segment!
>>> Realloc CodeSignature space... 
>>> Success!
>>> SignFolder: Frameworks/AppExtensions.framework, (AppExtensions)
>>> CodeResources regenerated with current icons
>>> SignFolder: Frameworks/Flutter.framework, (Flutter)
>>> CodeResources regenerated with current icons
>>> No enough CodeSignature space (now: 92160, need: 125000).
>>> Realloc CodeSignature space... 
>>> Success!
>>> SignFolder: Frameworks/otp_validator.framework, (otp_validator)
>>> CodeResources regenerated with current icons
>>> Can't find CodeSignature segment!
>>> Realloc CodeSignature space... 
>>> Success!
>>> SignFolder: Frameworks/image_picker.framework, (image_picker)
>>> CodeResources regenerated with current icons
>>> Can't find CodeSignature segment!
>>> Realloc CodeSignature space... 
>>> Success!
>>> SignFolder: Frameworks/device_info.framework, (device_info)
>>> CodeResources regenerated with current icons
>>> Can't find CodeSignature segment!
>>> Realloc CodeSignature space... 
>>> Success!
>>> SignFolder: Frameworks/ToastUI.framework, (ToastUI)
>>> CodeResources regenerated with current icons
>>> Can't find CodeSignature segment!
>>> Realloc CodeSignature space... 
>>> Success!
>>> SignFolder: Frameworks/PhotoPicker.framework, (PhotoPicker)
>>> CodeResources regenerated with current icons
>>> Can't find CodeSignature segment!
>>> Realloc CodeSignature space... 
>>> Success!
>>> SignFolder: Frameworks/package_manager.framework, (package_manager)
>>> CodeResources regenerated with current icons
>>> Can't find CodeSignature segment!
>>> Realloc CodeSignature space... 
>>> Success!
>>> SignFolder: Frameworks/web_view.framework, (web_view)
>>> CodeResources regenerated with current icons
>>> Can't find CodeSignature segment!
>>> Realloc CodeSignature space... 
>>> Success!
>>> SignFolder: Frameworks/Analytics.framework, (Analytics)
>>> CodeResources regenerated with current icons
>>> SignFolder: Frameworks/file_browser.framework, (file_browser)
>>> CodeResources regenerated with current icons
>>> Can't find CodeSignature segment!
>>> Realloc CodeSignature space... 
>>> Success!
>>> SignFolder: SampleApp.app, (Runner)
>>> CodeResources regenerated with current icons
>>> Can't find CodeSignature segment!
>>> Realloc CodeSignature space... 
>>> Success!
>>> Signed OK! (0.542s, 542198us)
>>> Archiving:  /home/appstore/output/signatures/xyz789AppSigned.ipa ... 
>>> Archive OK! (72.18 MB) (0.298s, 298564us)
>>> Done. (1.155s, 1155234us)
```
## Compile 

### macOS:

```bash
brew install pkg-config openssl minizip
git clone https://github.com/zhlynn/zsign.git
cd zsign/build/macos
make clean && make
```

Install `ideviceinstaller` for test:
```bash
brew install ideviceinstaller
```

### Linux:

#### Ubuntu 22.04 / Debian 12 / Mint 21:

```bash
sudo apt-get install -y git g++ pkg-config libssl-dev libminizip-dev
git clone https://github.com/zhlynn/zsign.git
cd zsign/build/linux
make clean && make
```

Install `ideviceinstaller` for test:
```bash
sudo apt-get install -y ideviceinstaller
```

#### RHEL / CentOS / Alma / Rocky / Other clones:

You must install `epel-release` first, eg:

RHEL / CentOS / Alma / Rocky 8:
```bash
sudo rpm -Uvh https://dl.fedoraproject.org/pub/epel/epel-release-latest-8.noarch.rpm
```

RHEL / CentOS / Alma / Rocky 9:
```bash
sudo rpm -Uvh https://dl.fedoraproject.org/pub/epel/epel-release-latest-9.noarch.rpm
```

Then, install the dependencies and compile:
```bash
sudo yum install -y git gcc-c++ pkg-config openssl-devel minizip1.2-devel
git clone https://github.com/zhlynn/zsign.git
cd zsign/build/linux
make clean && make
```

### Windows:

Use `Visual Studio 2022` to open `build/windows/vs2022/zsign.sln`, then compile it on Windows 10/11.
  
## Usage:

```bash
Usage: zsign [-options] [-k privkey.pem] [-m dev.prov] [-o output.ipa] file|folder
options:
-k, --pkey              Path to private key or p12 file. (PEM or DER format)
-m, --prov              Path to mobile provisioning profile.
-c, --cert              Path to certificate file. (PEM or DER format)
-a, --adhoc             Perform ad-hoc signature only.
-d, --debug             Generate debug output files. (.zsign_debug folder)
-f, --force             Force sign without cache when signing folder.
-o, --output            Path to output ipa file.
-p, --password          Password for private key or p12 file.
-b, --bundle_id         New bundle id to change.
-n, --bundle_name       New bundle name to change.
-r, --bundle_version    New bundle version to change.
-e, --entitlements      New entitlements to change.
-z, --zip_level         Compressed level when output the ipa file. (0-9)
-l, --dylib             Path to inject dylib file. Use -l multiple time to inject multiple dylib files at once.
-w, --weak              Inject dylib as LC_LOAD_WEAK_DYLIB.
-i, --install           Install ipa file using ideviceinstaller command for test.
-t, --temp_folder       Path to temporary folder for intermediate files.
-2, --sha256_only       Serialize a single code directory that uses SHA256.
-C, --check             Check if the file is signed.
-q, --quiet             Quiet operation.
-v, --version           Shows version.
-h, --help              Shows help (this message).
```

1. Show mach-o and codesignature segment info.
```bash
./zsign demo.app/demo
```

2. Sign ipa with private key and mobileprovisioning file.
```bash
./zsign -k privkey.pem -m dev.prov -o output.ipa -z 9 demo.ipa
```

3. Sign folder with p12 and mobileprovisioning file (using cache).
```bash
./zsign -k dev.p12 -p 123 -m dev.prov -o output.ipa demo.app
```

4. Sign folder with p12 and mobileprovisioning file (without cache).
```bash
./zsign -f -k dev.p12 -p 123 -m dev.prov -o output.ipa demo.app
```

5. Sign ipa with ad-hoc.
```bash
./zsign -a -o output.ipa demo.ipa
```

6. Inject dylib into ipa and re-sign.
```bash
./zsign -k dev.p12 -p 123 -m dev.prov -l demo.dylib -o output.ipa demo.ipa
```

7. Change bundle id and bundle name
```bash
./zsign -k dev.p12 -p 123 -m dev.prov -b 'com.new.bundle.id' -n 'NewName' -o output.ipa demo.ipa
```

8. Inject dylib(LC_LOAD_DYLIB) into mach-o file.
```bash
./zsign -a -l "@executable_path/demo1.dylib" -l "@executable_path/demo2.dylib" demo.app/execute
```

9. Inject dylib(LC_LOAD_WEAK_DYLIB) into mach-o file.
```bash
./zsign -w -l "@executable_path/demo.dylib" demo.app/execute
```

## How to sign quickly?

First, unzip the IPA file, then use zsign to sign the folder containing assets. During the initial signing, zsign will perform a complete signature and cache the signing information into a .zsign_cache directory in the current path. When re-signing the folder with different assets, zsign will utilize the cached data to significantly speed up the process—making signing extremely fast! Give it a try!

## License

zsign is licensed under the terms of MIT License. See the [LICENSE](LICENSE) file.
