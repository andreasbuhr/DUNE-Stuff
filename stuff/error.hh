#ifndef DUNE_STUFF_ERROR_HH
#define DUNE_STUFF_ERROR_HH

#include <dune/fem/misc/l2norm.hh>
#include <boost/format.hpp>
#include <utility>

namespace Stuff {
	template <class GridPartType >
	class L2Error {
			typedef Dune::L2Norm< GridPartType >
				L2NormType;
			L2NormType l2norm_;

		public:
			L2Error( const GridPartType& gridPart )
				: l2norm_( gridPart )
			{}

			struct Errors : public std::pair<double,double> {
				typedef std::pair<double,double>
					BaseType;
				double& absolute;
				double& relative;
				std::string name_;
				Errors( double abs, double rel, std::string name )
					: BaseType( abs, rel ),
					absolute( BaseType::first ),
					relative( BaseType::second ),
					name_(name)
				{}

				//!make friend op <<
				std::string str() const
				{
					return ( boost::format( "%s L2 error: %e (abs) | %e (rel)\n") % name_ % absolute % relative ).str();
				}
			};

			template < class DiscreteFunctionType >
			Errors get( const DiscreteFunctionType& function_A, const DiscreteFunctionType& function_B  ) const
			{
				DiscreteFunctionType tmp("L2Error::tmp", function_A.space() );
				tmp.assign( function_A );
				tmp -= function_B;
				const double abs = l2norm_.norm( tmp );
				return Errors ( abs,
								abs / l2norm_.norm( function_B ),
								function_A.name() );
			}

			template < class DiscreteFunctionType >
			Errors get( const DiscreteFunctionType& function_A, const DiscreteFunctionType& function_B, DiscreteFunctionType& diff ) const
			{
				diff.assign( function_A );
				diff -= function_B;
				const double abs = l2norm_.norm( diff );
				return Errors ( abs,
								abs / l2norm_.norm( function_B ),
								function_A.name()
								);
			}


	};

}//end namespace Stuff
#endif // DUNE_STUFF_ERROR_HH
