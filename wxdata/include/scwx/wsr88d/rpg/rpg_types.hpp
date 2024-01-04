#pragma once

namespace scwx
{
namespace wsr88d
{
namespace rpg
{

enum class DataLevelCode
{
   BadData,
   BelowThreshold,
   Blank,
   ChaffDetection,
   EditRemove,
   FlaggedData,
   Missing,
   NoData,
   OutsideCoverageArea,
   NoAccumulation,
   RangeFolded,
   Reserved,

   // Hydrometeor Classification
   Biological,
   AnomalousPropagationGroundClutter,
   IceCrystals,
   DrySnow,
   WetSnow,
   LightAndOrModerateRain,
   HeavyRain,
   BigDrops,
   Graupel,
   SmallHail,
   LargeHail,
   GiantHail,
   UnknownClassification,

   // Rainfall Rate Classification
   NoPrecipitation,
   Unfilled,
   Convective,
   Tropical,
   SpecificAttenuation,
   KL,
   KH,
   Z1,
   Z6,
   Z8,
   SI,

   Unknown
};

} // namespace rpg
} // namespace wsr88d
} // namespace scwx
