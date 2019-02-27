# nlimagemaker

Creates images that can be read and bootet by u-boot. Using this technique:

https://github.com/nonlinear-labs-dev/u-boot/commit/3fde2563258ae1f488ee737168f69f2c7cf51e61

## Usage

### Arguments

```
Usage: nlimagemaker [options]
  Image creation:
        -k, --kernel <kernel>        Kernel initramfs image (required)
        -d, --dts <dts>              Device Tree Blob (required)
        -r, --rootfs <rootfs>        Rootfs tarbal (required)
        -o, --output <target image>  Path to image (required)

  Image extraction:
        -e, --extract <img>          Image (required)

  Generic:
        -v, --verbose                Be more verbose
        -h, --help                   This help output
```

### Example

```
nlimagemaker -k initramfs.uImage -d devicetree.dtb -r rootfs.tar.gz -o nonlinear.img
```

## Building

```
git clone git@github.com:nonlinear-labs-dev/nlimagemaker.git
cd nlimagemaker && mkdir build && cd build && cmake ../ && make && make install
```
