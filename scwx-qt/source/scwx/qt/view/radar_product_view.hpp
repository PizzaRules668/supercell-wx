#pragma once

#include <scwx/common/color_table.hpp>
#include <scwx/qt/manager/radar_product_manager.hpp>

#include <chrono>
#include <memory>
#include <vector>

namespace scwx
{
namespace qt
{
namespace view
{

class RadarProductViewImpl;

class RadarProductView : public QObject
{
   Q_OBJECT

public:
   explicit RadarProductView(
      std::shared_ptr<manager::RadarProductManager> radarProductManager);
   ~RadarProductView();

   const std::vector<float>&                     vertices() const;
   const std::vector<boost::gil::rgba8_pixel_t>& color_table() const;

   void Initialize();

   std::tuple<const void*, size_t, size_t> GetMomentData();
   void LoadColorTable(std::shared_ptr<common::ColorTable> colorTable);

   std::chrono::system_clock::time_point SweepTime();

protected slots:
   void ComputeSweep();

signals:
   void ColorTableLoaded();
   void SweepComputed();

private:
   std::unique_ptr<RadarProductViewImpl> p;
};

} // namespace view
} // namespace qt
} // namespace scwx
