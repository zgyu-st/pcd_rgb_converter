# PCD RGB Convert

将包含独立 `red / green / blue` 字段的 PCD 点云转换为 PCL 标准的 `PointXYZRGB` 格式。

这个工具主要用于处理由 PDAL 从 LAS/LAZ 转换得到的 PCD 文件。

## 背景

原始 LAZ 点云包含：

* `X`
* `Y`
* `Z`
* `Intensity`
* `Red`
* `Green`
* `Blue`

使用 PDAL 直接转换为 PCD 后，颜色通常保存为独立字段：

```text
FIELDS x y z ... red green blue
```

这种格式能够保留 RGB 数据，但 PCL 的 `pcl_viewer` 和 `pcl::PointXYZRGB` 通常期望标准的打包 RGB 字段：

```text
FIELDS x y z rgb
```

本程序读取前一种 PCD，并输出标准的 `PointXYZRGB` PCD。

## RGB 转换

当前数据中的 RGB 为 16-bit 字段，但实际数据表现为 8-bit RGB 左移 8 位。

例如：

```text
Red   = 8192  -> 32
Green = 10752 -> 42
Blue  = 10240 -> 40
```

因此程序使用：

```cpp
r = red   >> 8;
g = green >> 8;
b = blue  >> 8;
```

将颜色恢复为 `0~255`。

## 依赖

测试环境：

* Ubuntu 22.04
* PCL 1.12.1
* CMake
* PDAL 2.3.0

安装 PCL 开发库：

```bash
sudo apt install libpcl-dev
```

安装 PDAL：

```bash
sudo apt install pdal libpdal-dev
```

## 编译

```bash
cmake -S . -B build
cmake --build build -j
```

生成可执行文件：

```text
build/pcd_rgb_convert
```

## 使用

```bash
./build/pcd_rgb_convert input.pcd output_rgb.pcd
```

例如：

```bash
./build/pcd_rgb_convert \
    ../rtlab_rgb16.pcd \
    ../rtlab_rgb.pcd
```

程序会输出转换的点数，例如：

```text
Converted 11878773 points
```

## 从 LAZ 开始的完整流程

首先使用 PDAL 将 LAZ 转成 PCD：

```bash
pdal translate \
    rtlab-20260915-100858.laz \
    rtlab_rgb16.pcd \
    --writers.pcd.compression=binary
```

PDAL 生成的 PCD 中 RGB 通常仍然是三个独立字段：

```text
red green blue
```

然后使用本工具转换为 PCL 标准的 `PointXYZRGB` 格式：

```bash
./build/pcd_rgb_convert \
    rtlab_rgb16.pcd \
    rtlab_rgb.pcd
```

最终输出的主要字段为：

```text
FIELDS x y z rgb
```

可以使用 PCL Viewer 查看：

```bash
pcl_viewer rtlab_rgb.pcd
```

## 输入要求

程序要求输入 PCD 至少包含以下字段：

```text
x
y
z
red
green
blue
```

程序会读取 XYZ 和独立 RGB 字段，并构造：

```cpp
pcl::PointCloud<pcl::PointXYZRGB>
```

最终使用 PCL 的 binary compressed PCD 格式保存。

## 注意事项

当前 RGB 转换逻辑针对本项目数据的编码方式：

```text
8-bit RGB << 8
```

因此使用：

```cpp
value >> 8
```

恢复 8-bit RGB。

如果其他 LAS/LAZ 数据真正使用完整的 16-bit RGB 范围，则可能需要修改 RGB 缩放方式，例如将 `0~65535` 映射到 `0~255`。

另外，在 Ubuntu 22.04 自带的 PCL 1.12.1 + VTK 9.1 环境中，`pcl_viewer` 在关闭窗口时可能出现 segmentation fault。

目前测试表明：

* PCD 可以正常加载
* XYZ 正常
* RGB 正常
* 点云能够正常显示

因此该问题更可能属于 PCL/VTK 可视化组件的退出问题，而不是 PCD 数据本身的问题。
