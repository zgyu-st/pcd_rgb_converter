#include <pcl/io/pcd_io.h>
#include <pcl/PCLPointCloud2.h>
#include <pcl/point_types.h>
#include <pcl/point_cloud.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>

double readField(const uint8_t* p, uint8_t datatype)
{
    switch (datatype)
    {
        case pcl::PCLPointField::FLOAT32: {
            float v;
            std::memcpy(&v, p, sizeof(v));
            return v;
        }
        case pcl::PCLPointField::FLOAT64: {
            double v;
            std::memcpy(&v, p, sizeof(v));
            return v;
        }
        case pcl::PCLPointField::UINT16: {
            uint16_t v;
            std::memcpy(&v, p, sizeof(v));
            return v;
        }
        case pcl::PCLPointField::UINT8: {
            uint8_t v;
            std::memcpy(&v, p, sizeof(v));
            return v;
        }
        default:
            throw std::runtime_error("Unsupported PCD field datatype");
    }
}

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0]
                  << " input.pcd output_rgb.pcd\n";
        return 1;
    }

    pcl::PCLPointCloud2 cloud;
    if (pcl::io::loadPCDFile(argv[1], cloud) < 0)
        return 1;

    const pcl::PCLPointField *fx=nullptr, *fy=nullptr, *fz=nullptr;
    const pcl::PCLPointField *fr=nullptr, *fg=nullptr, *fb=nullptr;

    for (const auto& f : cloud.fields)
    {
        if (f.name == "x") fx = &f;
        else if (f.name == "y") fy = &f;
        else if (f.name == "z") fz = &f;
        else if (f.name == "red") fr = &f;
        else if (f.name == "green") fg = &f;
        else if (f.name == "blue") fb = &f;
    }

    if (!fx || !fy || !fz || !fr || !fg || !fb)
    {
        std::cerr << "Missing x/y/z/red/green/blue fields\n";
        return 1;
    }

    const std::size_t n =
        static_cast<std::size_t>(cloud.width) * cloud.height;

    pcl::PointCloud<pcl::PointXYZRGB> out;
    out.resize(n);
    out.width = cloud.width;
    out.height = cloud.height;
    out.is_dense = cloud.is_dense;

    for (std::size_t i = 0; i < n; ++i)
    {
        const uint8_t* base = cloud.data.data() + i * cloud.point_step;

        auto& p = out[i];
        p.x = static_cast<float>(readField(base + fx->offset, fx->datatype));
        p.y = static_cast<float>(readField(base + fy->offset, fy->datatype));
        p.z = static_cast<float>(readField(base + fz->offset, fz->datatype));

        const auto r16 = static_cast<uint16_t>(readField(base + fr->offset, fr->datatype));
        const auto g16 = static_cast<uint16_t>(readField(base + fg->offset, fg->datatype));
        const auto b16 = static_cast<uint16_t>(readField(base + fb->offset, fb->datatype));

        p.r = static_cast<uint8_t>(std::min<uint16_t>(255, r16 >> 8));
        p.g = static_cast<uint8_t>(std::min<uint16_t>(255, g16 >> 8));
        p.b = static_cast<uint8_t>(std::min<uint16_t>(255, b16 >> 8));
    }

    if (pcl::io::savePCDFileBinaryCompressed(argv[2], out) < 0)
        return 1;

    std::cout << "Converted " << n << " points\n";
    return 0;
}
