# zsign

**zsign** is a fast, cross-platform codesign alternative for iOS 12+, supporting macOS, Linux, Windows, and more.  
If this tool helps you, please don't forget to 🌟 **star** 🌟 [ME](https://github.com/azozzalfiras/zsign).

---

## Example Usage

```bash
/usr/local/bin/zsign -k '/home/appstore/certificates/p12/Certificate_dev_1234567890_abc123.p12' \
    -m '/home/appstore/certificates/provision/Certificate_dev_profile_1234567890_def456.mobileprovision' \
    -o '/home/appstore/output/signatures/xyz789AppSigned.ipa' \
    -b '' -n '' '/home/appstore/builds/sample-app/Payload.ipa'
```

Sample output:
```logs
>>> Unzip: /home/appstore/builds/sample-app/Payload.ipa (28.45 MB) -> /tmp/zsign_folder_1234567890123456 ...
>>> Unzip OK! (0.387s, 387234us)
>>> Found Assets.car - removing to force icon regeneration...
>>> Removed old Assets.car - new icons will be used
>>> Signing: /tmp/zsign_folder_1234567890123456/Payload/SampleApp.app ...
>>> AppName: Sample App
>>> BundleId: dev.3zozz.sampleapp
>>> Version: 1.2
>>> TeamId: A1B2C3D4E5
>>> SubjectCN: iPhone Distribution: Dev 3zozz (A1B2C3D4E5)
...
>>> Signed OK! (0.542s, 542198us)
>>> Archiving: /home/appstore/output/signatures/xyz789AppSigned.ipa ...
>>> Archive OK! (72.18 MB) (0.298s, 298564us)
>>> Done. (1.155s, 1155234us)
```

---

## Compile

### macOS

```bash
brew install pkg-config openssl minizip
git clone https://github.com/AzozzALFiras/zsign.git
cd zsign/build/macos
make clean && make
```
Install for testing:
```bash
brew install ideviceinstaller
```

### Linux

#### Ubuntu / Debian / Mint

```bash
sudo apt-get install -y git g++ pkg-config libssl-dev libminizip-dev
git clone https://github.com/AzozzALFiras/zsign.git
cd zsign/build/linux
make clean && make
```
Install for testing:
```bash
sudo apt-get install -y ideviceinstaller
```

#### RHEL / CentOS / Alma / Rocky

Install `epel-release` first:
```bash
sudo rpm -Uvh https://dl.fedoraproject.org/pub/epel/epel-release-latest-8.noarch.rpm
# or for version 9:
sudo rpm -Uvh https://dl.fedoraproject.org/pub/epel/epel-release-latest-9.noarch.rpm
```
Then:
```bash
sudo yum install -y git gcc-c++ pkg-config openssl-devel minizip1.2-devel
git clone https://github.com/AzozzALFiras/zsign.git
cd zsign/build/linux
make clean && make
```

### Windows

Open `build/windows/vs2022/zsign.sln` in Visual Studio 2022 and build.

---

## Usage

```bash
Usage: zsign [-options] [-k privkey.pem] [-m dev.prov] [-o output.ipa] file|folder

Options:
    -k, --pkey              Path to private key or p12 file (PEM/DER)
    -m, --prov              Path to mobile provisioning profile
    -c, --cert              Path to certificate file (PEM/DER)
    -a, --adhoc             Perform ad-hoc signature only
    -d, --debug             Generate debug output files (.zsign_debug folder)
    -f, --force             Force sign without cache when signing folder
    -o, --output            Path to output ipa file
    -p, --password          Password for private key or p12 file
    -b, --bundle_id         New bundle id to change
    -n, --bundle_name       New bundle name to change
    -r, --bundle_version    New bundle version to change
    -e, --entitlements      New entitlements to change
    -z, --zip_level         Compressed level for output ipa (0-9)
    -l, --dylib             Path to inject dylib file (repeat for multiple)
    -w, --weak              Inject dylib as LC_LOAD_WEAK_DYLIB
    -i, --install           Install ipa using ideviceinstaller for test
    -t, --temp_folder       Path to temporary folder for intermediate files
    -2, --sha256_only       Serialize a single code directory using SHA256
    -C, --check             Check if the file is signed
    -q, --quiet             Quiet operation
    -v, --version           Show version
    -h, --help              Show help
```

### Common Commands

- Show Mach-O and codesignature info:
    ```bash
    ./zsign demo.app/demo
    ```
- Sign IPA with private key and provisioning profile:
    ```bash
    ./zsign -k privkey.pem -m dev.prov -o output.ipa -z 9 demo.ipa
    ```
- Sign folder with p12 and provisioning profile (with cache):
    ```bash
    ./zsign -k dev.p12 -p 123 -m dev.prov -o output.ipa demo.app
    ```
- Sign folder without cache:
    ```bash
    ./zsign -f -k dev.p12 -p 123 -m dev.prov -o output.ipa demo.app
    ```
- Ad-hoc sign IPA:
    ```bash
    ./zsign -a -o output.ipa demo.ipa
    ```
- Inject dylib and re-sign:
    ```bash
    ./zsign -k dev.p12 -p 123 -m dev.prov -l demo.dylib -o output.ipa demo.ipa
    ```
- Change bundle id and name:
    ```bash
    ./zsign -k dev.p12 -p 123 -m dev.prov -b 'com.new.bundle.id' -n 'NewName' -o output.ipa demo.ipa
    ```
- Inject multiple dylibs:
    ```bash
    ./zsign -a -l "@executable_path/demo1.dylib" -l "@executable_path/demo2.dylib" demo.app/execute
    ```
- Inject weak dylib:
    ```bash
    ./zsign -w -l "@executable_path/demo.dylib" demo.app/execute
    ```

---

## Fast Signing

Unzip the IPA, then use zsign to sign the folder containing assets. The first signing creates a `.zsign_cache` directory. When re-signing with different assets, zsign uses the cache for much faster signing.

---

## License

zsign is licensed under the MIT License. See the [LICENSE](LICENSE) file.
