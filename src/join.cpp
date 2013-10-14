// Copyright (C) 2013    Romain Francois
// Copyright (C) 2013    Rice University
//
// This file is part of dplyr.
//
// dplyr is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// dplyr is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with dplyr.  If not, see <http://www.gnu.org/licenses/>.

#include <dplyr.h>

using namespace Rcpp ;
using namespace dplyr ;

CharacterVector common_by( CharacterVector x, CharacterVector y){
    return intersect(x, y) ;    
}

DataFrame subset( DataFrame df, IntegerVector indices, CharacterVector columns){
    DataFrameVisitors visitors(df, columns) ;
    return visitors.copy(indices) ;
}

// [[Rcpp::export]]
DataFrame semi_join_impl( DataFrame x, DataFrame y){
    typedef VisitorSetIndexMap<DataFrameJoinVisitors, std::vector<int> > Map ;
    CharacterVector by   = common_by(x.names(), y.names()) ;
    DataFrameJoinVisitors visitors(x, y, by) ;
    Map map(visitors);  
    
    // train the map in terms of x
    int n_x = x.nrows() ;
    for( int i=0; i<n_x; i++)
        map[i].push_back(i) ;
    
    int n_y = y.nrows() ;
    std::vector<int> indices ;
    for( int i=0; i<n_y; i++){
        Map::iterator it = map.find(-i-1) ;
        if( it != map.end() ){
            std::vector<int>& chunk = it->second ;
            indices.insert( indices.end(), chunk.begin(), chunk.end() ) ;
            map.erase(it) ;
        }
    }
    
    // TODO: now we need to give an IntegerVector to subset
    //       we should be able to give the std::vector<int> instead
    return subset(x, IntegerVector(wrap(indices)), x.names() ) ;
}

// [[Rcpp::export]]
DataFrame anti_join_impl( DataFrame x, DataFrame y){
    typedef VisitorSetIndexMap<DataFrameJoinVisitors, std::vector<int> > Map ;
    CharacterVector by   = common_by(x.names(), y.names()) ;
    DataFrameJoinVisitors visitors(x, y, by) ;
    Map map(visitors);  
    
    // train the map in terms of x
    int n_x = x.nrows() ;
    for( int i=0; i<n_x; i++)
        map[i].push_back(i) ;
    
    int n_y = y.nrows() ;
    // remove the rows in x that match
    for( int i=0; i<n_y; i++){
        Map::iterator it = map.find(-i-1) ;
        if( it != map.end() )
            map.erase(it) ;
    }
    
    // collect what's left
    std::vector<int> indices ;
    Map::iterator it = map.begin() ;
    for( ; it != map.end(); ++it){
        std::vector<int>& chunk = it->second ;
        indices.insert( indices.end(), chunk.begin(), chunk.end() ) ;    
    }
    
    // TODO: now we need to give an IntegerVector to subset
    //       we should be able to give the std::vector<int> instead
    return subset(x, IntegerVector(wrap(indices)), x.names() ) ;
}


