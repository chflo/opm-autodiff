/*
  Copyright 2015 Dr. Blatt - HPC-Simulation-Software & Services.
  Coypright 2015 NTNU
  Copyright 2015 Statoil AS
  Copyright 2015 IRIS AS

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
#ifndef OPM_REDISTRIBUTEDATAHANDLES_HEADER
#define OPM_REDISTRIBUTEDATAHANDLES_HEADER

#include <opm/core/simulator/BlackoilState.hpp>

#include <opm/autodiff/BlackoilPropsAdFromDeck.hpp>
#include <opm/autodiff/ExtractParallelGridInformationToISTL.hpp>

#include<boost/any.hpp>

namespace Opm
{

template <class Grid>
inline void distributeGridAndData( Grid& ,
                                   Opm::DeckConstPtr ,
                                   EclipseStateConstPtr ,
                                   BlackoilState& ,
                                   BlackoilPropsAdFromDeck& ,
                                   DerivedGeology&,
                                   std::shared_ptr<BlackoilPropsAdFromDeck::MaterialLawManager>&,
                                   std::vector<double>&,
                                   boost::any& ,
                                   const bool )
{
}

#if HAVE_DUNE_CORNERPOINT && HAVE_MPI
/// \brief a data handle to distribute the threshold pressures
class ThresholdPressureDataHandle
{
public:
    /// \brief type of the data we send
    typedef double DataType;
    /// \brief Constructor
    /// \param sendGrid    The grid that the data is attached to when sending.
    /// \param recvGrid    The grid that the data is attached to when receiving.
    /// \param sendPressures The container where we will retrieve the values to be sent.
    /// \param numFaces Number of faces of the distributed grid.
    ThresholdPressureDataHandle(const Dune::CpGrid& sendGrid,
                                const Dune::CpGrid& recvGrid,
                                const std::vector<double>& sendPressures,
                                std::vector<double>& recvPressures)
        : sendGrid_(sendGrid), recvGrid_(recvGrid), sendPressures_(sendPressures),
          recvPressures_(recvPressures)
    {}

    bool fixedsize(int /*dim*/, int /*codim*/)
    {
        return false;
    }
    template<class T>
    std::size_t size(const T& e)
    {
        if ( T::codimension == 0)
        {
            return sendGrid_.numCellFaces(e.index());
        }
        else
        {
            OPM_THROW(std::logic_error, "Data handle can only be used for elements");
        }
    }
    template<class B, class T>
    void gather(B& buffer, const T& e)
    {
        assert( T::codimension == 0);
        for ( int i=0; i< sendGrid_.numCellFaces(e.index()); ++i )
        {
            buffer.write(sendPressures_[sendGrid_.cellFace(e.index(), i)]);
        }
    }
    template<class B, class T>
    void scatter(B& buffer, const T& e, std::size_t /* size */)
    {
        assert( T::codimension == 0);
        for ( int i=0; i< recvGrid_.numCellFaces(e.index()); ++i )
        {
            double val;
            buffer.read(val);
            recvPressures_[recvGrid_.cellFace(e.index(), i)]=val;
        }
    }
    bool contains(int dim, int codim)
    {
        return dim==3 && codim==0;
    }

private:
    /// \brief The grid that the data we send is associated with.
    const Dune::CpGrid& sendGrid_;
    /// \brief The grid that the data we receive is associated with.
    const Dune::CpGrid& recvGrid_;
    /// \brief The data to send.
    const std::vector<double>& sendPressures_;
    /// \brief The data to receive.
    std::vector<double>& recvPressures_;
};

/// \brief a data handle to distribute Derived Geology
class GeologyDataHandle
{
public:
    /// \brief type of the data we send
    typedef double DataType;
    /// \brief Constructor
    /// \param sendGrid    The grid that the data is attached to when sending.
    /// \param recvGrid    The grid that the data is attached to when receiving.
    /// \param sendGeology The state where we will retieve the values to be sent.
    /// \param recvGeology The state where we will store the received values.
    GeologyDataHandle(const Dune::CpGrid& sendGrid,
                      const Dune::CpGrid& recvGrid,
                      const DerivedGeology& sendGeology,
                      DerivedGeology& recvGeology)
        : sendGrid_(sendGrid), recvGrid_(recvGrid), sendGeology_(sendGeology),
          recvGeology_(recvGeology)
    {}

    bool fixedsize(int /*dim*/, int /*codim*/)
    {
        return false;
    }
    template<class T>
    std::size_t size(const T& e)
    {
        if ( T::codimension == 0)
        {
            return 1 + sendGrid_.numCellFaces(e.index());
        }
        else
        {
            OPM_THROW(std::logic_error, "Data handle can only be used for elements");
        }
    }
    template<class B, class T>
    void gather(B& buffer, const T& e)
    {
        assert( T::codimension == 0);
        buffer.write(sendGeology_.poreVolume()[e.index()]);
        for ( int i=0; i< sendGrid_.numCellFaces(e.index()); ++i )
        {
            buffer.write(sendGeology_.transmissibility()[sendGrid_.cellFace(e.index(), i)]);
        }
    }
    template<class B, class T>
    void scatter(B& buffer, const T& e, std::size_t /* size */)
    {
        assert( T::codimension == 0);
        double val;
        buffer.read(val);
        recvGeology_.poreVolume()[e.index()]=val;
        for ( int i=0; i< recvGrid_.numCellFaces(e.index()); ++i )
        {
            buffer.read(val);
            recvGeology_.transmissibility()[recvGrid_.cellFace(e.index(), i)]=val;
        }
    }
    bool contains(int dim, int codim)
    {
        return dim==3 && codim==0;
    }
private:
    /// \brief The grid that the data we send is associated with.
    const Dune::CpGrid& sendGrid_;
    /// \brief The grid that the data we receive is associated with.
    const Dune::CpGrid& recvGrid_;
    /// \brief The data to send.
    const DerivedGeology& sendGeology_;
    /// \brief The data to receive.
    DerivedGeology& recvGeology_;
};

/// \brief a data handle to distribute the BlackoilState
class BlackoilStateDataHandle
{
public:
    /// \brief The data that we send.
    typedef double DataType;
    /// \brief Constructor.
    /// \param sendGrid   The grid that the data is attached to when sending.
    /// \param recvGrid   The grid that the data is attached to when receiving.
    /// \param sendState  The state where we will retieve the values to be sent.
    /// \param recvState The state where we will store the received values.
    BlackoilStateDataHandle(const Dune::CpGrid& sendGrid,
                            const Dune::CpGrid& recvGrid,
                            const BlackoilState& sendState,
                            BlackoilState& recvState)
        : sendGrid_(sendGrid), recvGrid_(recvGrid), sendState_(sendState), recvState_(recvState)
    {}

    bool fixedsize(int /*dim*/, int /*codim*/)
    {
        return false;
    }

    template<class T>
    std::size_t size(const T& e)
    {
        if ( T::codimension == 0)
        {
            return 2 * sendState_.numPhases() +4+2*sendGrid_.numCellFaces(e.index());
        }
        else
        {
            OPM_THROW(std::logic_error, "Data handle can only be used for elements");
        }
    }

    template<class B, class T>
    void gather(B& buffer, const T& e)
    {
        assert( T::codimension == 0);

        for ( size_t i=0; i<sendState_.numPhases(); ++i )
        {
            buffer.write(sendState_.surfacevol()[e.index()*sendState_.numPhases()+i]);
        }
        buffer.write(sendState_.gasoilratio()[e.index()]);
        buffer.write(sendState_.rv()[e.index()]);
        buffer.write(sendState_.pressure()[e.index()]);
        buffer.write(sendState_.temperature()[e.index()]);
        for ( size_t i=0; i<sendState_.numPhases(); ++i )
        {
            buffer.write(sendState_.saturation()[e.index()*sendState_.numPhases()+i]);
        }
        for ( int i=0; i<sendGrid_.numCellFaces(e.index()); ++i )
        {
            buffer.write(sendState_.facepressure()[sendGrid_.cellFace(e.index(), i)]);
        }
        for ( int i=0; i<sendGrid_.numCellFaces(e.index()); ++i )
        {
            buffer.write(sendState_.faceflux()[sendGrid_.cellFace(e.index(), i)]);
        }
    }
    template<class B, class T>
    void scatter(B& buffer, const T& e, std::size_t size_arg)
    {
        assert( T::codimension == 0);
        assert( size_arg == 2 * recvState_.numPhases() +4+2*recvGrid_.numCellFaces(e.index()));
        static_cast<void>(size_arg);

        double val;
        for ( size_t i=0; i<recvState_.numPhases(); ++i )
        {
            buffer.read(val);
            recvState_.surfacevol()[e.index()*sendState_.numPhases()+i]=val;
        }
        buffer.read(val);
        recvState_.gasoilratio()[e.index()]=val;
        buffer.read(val);
        recvState_.rv()[e.index()]=val;
        buffer.read(val);
        recvState_.pressure()[e.index()]=val;
        buffer.read(val);
        recvState_.temperature()[e.index()]=val;
        for ( size_t i=0; i<recvState_.numPhases(); ++i )
        {
            buffer.read(val);
            recvState_.saturation()[e.index()*sendState_.numPhases()+i]=val;
        }
        for ( int i=0; i<recvGrid_.numCellFaces(e.index()); ++i )
        {
            buffer.read(val);
            recvState_.facepressure()[recvGrid_.cellFace(e.index(), i)]=val;
        }
        for ( int i=0; i<recvGrid_.numCellFaces(e.index()); ++i )
        {
            buffer.read(val);
            recvState_.faceflux()[recvGrid_.cellFace(e.index(), i)]=val;
        }
    }
    bool contains(int dim, int codim)
    {
        return dim==3 && codim==0;
    }
private:
    /// \brief The grid that the data is attached to when sending
    const Dune::CpGrid& sendGrid_;
    /// \brief The grid that the data is attached to when receiving
    const Dune::CpGrid& recvGrid_;
    /// \brief The state where we will retieve the values to be sent.
    const BlackoilState& sendState_;
    // \brief The state where we will store the received values.
    BlackoilState& recvState_;
};

/// \brief A DUNE data handle for sending the blackoil properties
class BlackoilPropsDataHandle
{
public:
    /// \brief The data that we send.
    typedef double DataType;
    /// \brief Constructor.
    /// \param sendProps  The properties where we will retieve the values to be sent.
    /// \parame recvProps The properties where we will store the received values.
    BlackoilPropsDataHandle(const BlackoilPropsAdFromDeck& sendProps,
                            BlackoilPropsAdFromDeck& recvProps)
        : sendProps_(sendProps), recvProps_(recvProps),
          size_(11) // full permeability tensor 9 + porosity 1 + pvt region index
    {
        // satOilMax might be non empty. In this case we will need to send it, too.
        if ( sendProps.satOilMax_.size()>0 )
        {

            // satOilMax has to have the same size as the cellPvtRegionIdx_
            recvProps_.satOilMax_.resize(recvProps_.cellPvtRegionIdx_.size(),
                                         -std::numeric_limits<double>::max());
            ++size_;
        }
    }

    bool fixedsize(int /*dim*/, int /*codim*/)
    {
        return true;
    }

    template<class T>
    std::size_t size(const T&)
    {
        if ( T::codimension == 0)
        {
            return size_;
        }
        else
        {
            OPM_THROW(std::logic_error, "Data handle can only be used for elements");
        }
    }
    template<class B, class T>
    void gather(B& buffer, const T& e)
    {
        assert( T::codimension == 0);

        buffer.write(sendProps_.cellPvtRegionIndex()[e.index()]);
        for( std::size_t i = 0; i < 9; ++i )
        {
            buffer.write(sendProps_.rock_.permeability_[e.index()*9+i]);
        }
        buffer.write(sendProps_.rock_.porosity_[e.index()]);
        if ( size_ > 11 ) {
            buffer.write(sendProps_.satOilMax_[e.index()]);
        }
    }
    template<class B, class T>
    void scatter(B& buffer, const T& e, std::size_t size_arg)
    {
        assert( T::codimension == 0);
        assert( size_arg==size_ ); (void) size_arg;
        double val;
        buffer.read(val);
        recvProps_.cellPvtRegionIdx_[e.index()]=val;
        for( std::size_t i = 0; i < 9; ++i )
        {
            buffer.read(val);
            recvProps_.rock_.permeability_[e.index()*9+i]
                = val;
        }
        buffer.read(val);
        recvProps_.rock_.porosity_[e.index()]=val;
        if ( size_ > 11 ) {
            buffer.read(val);
            recvProps_.satOilMax_[e.index()]=val;
        }
    }
    bool contains(int dim, int codim)
    {
        return dim==3 && codim==0;
    }
private:
    /// \brief The properties where we will retieve the values to be sent.
    const BlackoilPropsAdFromDeck& sendProps_;
    /// \brief The properties where we will store the received values.
    BlackoilPropsAdFromDeck& recvProps_;
    /// \brief The number of entries to send.
    ///
    /// full permeability tensor 9 + porosity 1 + pvt region index and
    /// in some case satOilMax
    std::size_t size_;
};

inline
void distributeGridAndData( Dune::CpGrid& grid,
                            Opm::DeckConstPtr deck,
                            EclipseStateConstPtr eclipseState,
                            BlackoilState& state,
                            BlackoilPropsAdFromDeck& properties,
                            DerivedGeology& geology,
                            std::shared_ptr<BlackoilPropsAdFromDeck::MaterialLawManager>& material_law_manager,
                            std::vector<double>& threshold_pressures,
                            boost::any& parallelInformation,
                            const bool useLocalPerm)
{
    Dune::CpGrid global_grid ( grid );
    global_grid.switchToGlobalView();

    // distribute the grid and switch to the distributed view
    grid.loadBalance(eclipseState, geology.transmissibility().data());
    grid.switchToDistributedView();
    std::vector<int> compressedToCartesianIdx;
    Opm::createGlobalCellArray(grid, compressedToCartesianIdx);
    typedef BlackoilPropsAdFromDeck::MaterialLawManager MaterialLawManager;
    auto distributed_material_law_manager = std::make_shared<MaterialLawManager>();
    distributed_material_law_manager->initFromDeck(deck, eclipseState, compressedToCartesianIdx);
    // copy the values from the global to the local MaterialLawManager
    // We should actually communicate these to be future proof. But that is
    // really, really cumbersome for the underlying vector<shared_ptr>
    // where the classes pointed to even have more shared_ptr stored in them.
    typedef Dune::CpGrid::ParallelIndexSet IndexSet;
    const IndexSet& local_indices  = grid.getCellIndexSet();
    for ( auto index : local_indices )
    {
        distributed_material_law_manager->materialLawParamsPointer(index.local()) =
            material_law_manager->materialLawParamsPointer(index.global());

        distributed_material_law_manager->oilWaterScaledEpsInfoDrainagePointer(index.local()) =
            material_law_manager->oilWaterScaledEpsInfoDrainagePointer(index.global());
    }
    BlackoilPropsAdFromDeck distributed_props(properties,
                                              distributed_material_law_manager,
                                              grid.numCells());
    BlackoilState distributed_state(grid.numCells(), grid.numFaces(), state.numPhases());
    // construction does not resize surfacevol. Do it manually.
    distributed_state.surfacevol().resize(grid.numCells()*state.numPhases(),
                                          std::numeric_limits<double>::max());
    BlackoilStateDataHandle state_handle(global_grid, grid,
                                         state, distributed_state);
    BlackoilPropsDataHandle props_handle(properties,
                                         distributed_props);
    grid.scatterData(state_handle);
    grid.scatterData(props_handle);
    // Create a distributed Geology. Some values will be updated using communication
    // below
    DerivedGeology distributed_geology(grid,
                                       distributed_props, eclipseState,
                                       useLocalPerm, geology.gravity());
    GeologyDataHandle geo_handle(global_grid, grid,
                                 geology, distributed_geology);
    grid.scatterData(geo_handle);

    std::vector<double> distributed_pressures;

    if( !threshold_pressures.empty() ) // Might be empty if not specified
    {
        if( threshold_pressures.size() !=
            static_cast<std::size_t>(UgGridHelpers::numFaces(global_grid)) )
        {
            OPM_THROW(std::runtime_error, "NNCs not yet supported for parallel runs. "
                      << UgGridHelpers::numFaces(grid) << " faces but " <<
                      threshold_pressures.size()<<" threshold pressure values");
        }
        distributed_pressures.resize(UgGridHelpers::numFaces(grid));
        ThresholdPressureDataHandle press_handle(global_grid, grid,
                                                 threshold_pressures,
                                                 distributed_pressures);
        grid.scatterData(press_handle);
    }

    // copy states
    properties           = distributed_props;
    geology              = distributed_geology;
    state                = distributed_state;
    material_law_manager = distributed_material_law_manager;
    threshold_pressures   = distributed_pressures;
    extractParallelGridInformationToISTL(grid, parallelInformation);
}
#endif

} // end namespace Opm
#endif
