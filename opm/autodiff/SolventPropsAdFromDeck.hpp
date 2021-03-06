/*
  Copyright 2015 IRIS
  
  This file is part of the Open Porous Media project (OPM).
  
  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SOLVENTPROPSADFROMDECK_HPP
#define SOLVENTPROPSADFROMDECK_HPP

#include <opm/autodiff/BlackoilPropsAdInterface.hpp>
#include <opm/autodiff/AutoDiffBlock.hpp>

#include <opm/core/utility/NonuniformTableLinear.hpp>

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>

#include <cmath>
#include <vector>
#include <opm/common/ErrorMacros.hpp>

namespace Opm
{
class SolventPropsAdFromDeck
{
public:
    SolventPropsAdFromDeck(DeckConstPtr deck,
                           EclipseStateConstPtr eclipseState,
                           const int number_of_cells,
                           const int* global_cell);

    ////////////////////////////
    //      Fluid interface   //
    ////////////////////////////

    typedef AutoDiffBlock<double> ADB;
    typedef ADB::V V;
    typedef std::vector<int> Cells;

    /// Solvent formation volume factor.
    /// \param[in]  pg     Array of n gas pressure values.
    /// \param[in]  cells  Array of n cell indices to be associated with the pressure values.
    /// \return            Array of n formation volume factor values.
    ADB bSolvent(const ADB& pg,
                 const Cells& cells) const;

    /// Solvent viscosity.
    /// \param[in]  pg     Array of n gas pressure values.
    /// \param[in]  cells  Array of n cell indices to be associated with the pressure values.
    /// \return            Array of n viscosity values.
    ADB muSolvent(const ADB& pg,
                  const Cells& cells) const;

    /// Gas relPerm multipliers
    /// \param[in]  gasFraction     Array of n gas fraction Sg / (sg + Ss) values.
    /// \param[in]  cells           Array of n cell indices to be associated with the fraction values.
    /// \return                     Array of n gas relPerm multiplier values.
    ADB gasRelPermMultiplier(const ADB& solventFraction,
                             const Cells& cells) const;

    /// Solvent relPerm multipliers
    /// \param[in]  solventFraction Array of n solvent fraction Ss / (Sg + Ss) values.
    /// \param[in]  cells           Array of n cell indices to be associated with the fraction values.
    /// \return                     Array of n solvent relPerm multiplier values.
    ADB solventRelPermMultiplier(const ADB& solventFraction,
                                 const Cells& cells) const;

    /// Miscible hydrocrabon relPerm wrt water
    /// \param[in]  Sn              Array of n total hyrdrocarbon saturation values.
    /// \param[in]  cells           Array of n cell indices to be associated with the fraction values.
    /// \return                     Array of n miscible hyrdrocabon wrt water relPerm values.
    ADB misicibleHydrocarbonWaterRelPerm(const ADB& Sn,
                                         const Cells& cells) const;

    /// Miscible Solvent + Gas relPerm multiplier
    /// \param[in]  Ssg             Array of n total gas fraction (Sgas + Ssolvent) / Sn values, where
    ///                             Sn = Sgas + Ssolvent + Soil.
    /// \param[in]  cells           Array of n cell indices to be associated with the fraction values.
    /// \return                     Array of n solvent gas relperm multiplier.
    ADB miscibleSolventGasRelPermMultiplier(const ADB& Ssg,
                                            const Cells& cells) const;

    /// Miscible Oil relPerm multiplier
    /// \param[in]  So              Array of n oil fraction values. Soil / Sn values, where Sn = Sgas + Ssolvent + Soil.
    /// \param[in]  cells           Array of n cell indices to be associated with the fraction values.
    /// \return                     Array of n oil relperm multiplier.
    ADB miscibleOilRelPermMultiplier(const ADB& So,
                                     const Cells& cells) const;

    /// Miscible function
    /// \param[in]  solventFraction Array of n solvent fraction Ss / (Sg + Ss) values.
    /// \param[in]  cells           Array of n cell indices to be associated with the fraction values.
    /// \return                     Array of n miscibility values
    ADB miscibilityFunction(const ADB& solventFraction,
                            const Cells& cells) const;

    /// Pressure dependent miscibility function
    /// \param[in]  solventFraction Array of n oil phase pressure .
    /// \param[in]  cells           Array of n cell indices to be associated with the pressure values.
    /// \return                     Array of n miscibility values
    ADB pressureMiscibilityFunction(const ADB& po,
                                    const Cells& cells) const;

    /// Miscible critical gas saturation function
    /// \param[in]  Sw              Array of n water saturation values.
    /// \param[in]  cells           Array of n cell indices to be associated with the saturation values.
    /// \return                     Array of n miscible critical gas saturation values
    ADB miscibleCriticalGasSaturationFunction(const ADB& Sw,
                                              const Cells& cells) const;

    /// Miscible residual oil saturation function
    /// \param[in]  Sw              Array of n water saturation values.
    /// \param[in]  cells           Array of n cell indices to be associated with the saturation values.
    /// \return                     Array of n miscible residual oil saturation values
    ADB miscibleResidualOilSaturationFunction(const ADB& Sw,
                                              const Cells& cells) const;

    /// Solvent surface density
    /// \param[in]  cells           Array of n cell indices to be associated with the fraction values.
    /// \return                     Array of n solvent density values.
    V solventSurfaceDensity(const Cells& cells) const;

    /// Todd-Longstaff mixing parameter for viscosity calculation
    /// \param[in]  cells           Array of n cell indices to be associated with the fraction values.
    /// return                      Array of n mixing paramters for viscosity calculation
    V mixingParameterViscosity(const Cells& cells) const;

    /// Todd-Longstaff mixing parameter for density calculation
    /// \param[in]  cells           Array of n cell indices to be associated with the fraction values.
    /// return                      Array of n mixing paramters for density calculation
    V mixingParameterDensity(const Cells& cells) const;


private:
    /// Makes ADB from table values
    /// \param[in]  X               Array of n table lookup values.
    /// \param[in]  cells           Array of n cell indices to be associated with the fraction values.
    /// \param[in]  tables          Vector of tables, one for each PVT region.
    /// \return                     Array of n solvent density values.
    ADB makeADBfromTables(const ADB& X,
                          const Cells& cells,
                          const std::vector<int>& regionIdx,
                          const std::vector<NonuniformTableLinear<double>>& tables) const;

    /// Helper function to create an array containing the
    /// table index of for each compressed cell from an Eclipse deck.
    /// \param[in] keyword      eclKeyword specifying region (SATNUM etc. )
    /// \param[in] eclState     eclState from opm-parser
    /// \param[in] numCompressed number of compressed cells
    /// \param[in] compressedToCartesianCellIdx cartesianCellIdx for each cell in the grid
    /// \param[out] tableIdx table index for each compressed cell
    void extractTableIndex(const std::string& keyword,
                           Opm::EclipseStateConstPtr eclState,
                           size_t numCompressed,
                           const int* compressedToCartesianCellIdx,
                           std::vector<int>& tableIdx) const;

    // The PVT region which is to be used for each cell
    std::vector<int> cellPvtRegionIdx_;
    std::vector<int> cellMiscRegionIdx_;
    std::vector<int> cellSatNumRegionIdx_;
    std::vector<NonuniformTableLinear<double> > b_;
    std::vector<NonuniformTableLinear<double> > viscosity_;
    std::vector<NonuniformTableLinear<double> > inverseBmu_;
    std::vector<double> solvent_surface_densities_;
    std::vector<NonuniformTableLinear<double> > krg_;
    std::vector<NonuniformTableLinear<double> > krs_;
    std::vector<NonuniformTableLinear<double> > krn_;
    std::vector<NonuniformTableLinear<double> > mkro_;
    std::vector<NonuniformTableLinear<double> > mkrsg_;
    std::vector<NonuniformTableLinear<double> > misc_;
    std::vector<NonuniformTableLinear<double> > pmisc_;
    std::vector<NonuniformTableLinear<double> > sorwmis_;
    std::vector<NonuniformTableLinear<double> > sgcwmis_;
    std::vector<double> mix_param_viscosity_;
    std::vector<double> mix_param_density_;
};

} // namespace OPM

#endif // SOLVENTPROPSADFROMDECK_HPP
