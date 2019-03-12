// ===============================================================================
// Authors: AFRL/RQQA
// Organization: Air Force Research Laboratory, Aerospace Systems Directorate, Power and Control Division
// 
// Copyright (c) 2017 Government of the United State of America, as represented by
// the Secretary of the Air Force.  No copyright is claimed in the United States under
// Title 17, U.S. Code.  All Other Rights Reserved.
// ===============================================================================

//
//    THIS SOFTWARE AND ANY ACCOMPANYING DOCUMENTATION IS RELEASED "AS IS." THE
//    U.S.GOVERNMENT MAKES NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, CONCERNING
//    THIS SOFTWARE AND ANY ACCOMPANYING DOCUMENTATION, INCLUDING, WITHOUT LIMITATION,
//    ANY WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT
//    WILL THE U.S. GOVERNMENT BE LIABLE FOR ANY DAMAGES, INCLUDING ANY LOST PROFITS,
//    LOST SAVINGS OR OTHER INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE,
//    OR INABILITY TO USE, THIS SOFTWARE OR ANY ACCOMPANYING DOCUMENTATION, EVEN IF
//    INFORMED IN ADVANCE OF THE POSSIBILITY OF SUCH DAMAGES.
//
//    CGrid class.
//
//////////////////////////////////////////////////////////////////////
//    Date    Name    Ver     Modification
//    3/4/05    afj     1.0     created
//
//////////////////////////////////////////////////////////////////////

//#pragma warning(disable:4786)
#define DEBUG_FLAG 0
#define DEBUG_TEST 0
#define METHOD_DEBUG 0

//#define PLANE 1

#include "CGrid.h"
#include <iostream>
#include <sstream>
#include <math.h>
//#include "mex.h"

#ifdef LINUX
#include <stdio.h>
#endif    //LINUX

using namespace std;

namespace n_FrameworkLib
{


/****************************************************/
/*        Constructors                                 */
/****************************************************/
CGrid::CGrid(std::vector<int32_t>& viVerticies,V_POSITION_t& vposVertexContainer, int x_grid_resolution, 
             int y_grid_resolution, CPosition &min, CPosition &max,
             stringstream &sstrErrorMessage):
//VGrid(0), 
//Index(0),
//UpperBound(0),
//Polygon_Points (polygon_points),
xdelta(0),
ydelta(0),
inv_xdelta(0),
inv_ydelta(0),
XGridResolution(x_grid_resolution),
YGridResolution(y_grid_resolution),
TotalCells(x_grid_resolution*y_grid_resolution),
NearEpsilon(1e-9),
dHUGE(1.797693134862315e+308),
GC_BL_IN(0x0001),
GC_BR_IN(0x0002),
GC_TL_IN(0x0004),
GC_TR_IN(0x0008),
GC_L_EDGE_HIT(0x0010),
GC_R_EDGE_HIT(0x0020),
GC_B_EDGE_HIT(0x0040),
GC_T_EDGE_HIT(0x0080),
GC_B_EDGE_PARITY(0x0100),
GC_T_EDGE_PARITY(0x0200),
GC_AIM_L((0<<10)),
GC_AIM_B((1<<10)),
GC_AIM_R((2<<10)),
GC_AIM_T((3<<10)),
GC_AIM_C((4<<10)),
GC_AIM(0x1c00),
GC_L_EDGE_CLEAR(GC_L_EDGE_HIT),
GC_R_EDGE_CLEAR(GC_R_EDGE_HIT),
GC_B_EDGE_CLEAR(GC_B_EDGE_HIT),
GC_T_EDGE_CLEAR(GC_T_EDGE_HIT)
{
#if DEBUG_FLAG==1
    cout << "CGRid constructor Method\n";
#endif
    
    VGrid.resize(TotalCells);                    // resize array - grab all memory

    glx.resize(XGridResolution+1);
    gly.resize(YGridResolution+1);
    int attempt = 0;
    
TryAgain:
    
    xdelta = (max.m_north_m - min.m_north_m)/XGridResolution;
    inv_xdelta = 1.0 / xdelta;
    ydelta = (max.m_east_m - min.m_east_m)/YGridResolution;
    inv_ydelta = 1.0 / ydelta;
    
    
    int i =0;
    for (; i< XGridResolution ; i++) 
        glx[i] = min.m_north_m + i * xdelta;
    glx[i] = max.m_north_m;
    
    for (i =0; i< YGridResolution ; i++) 
        gly[i] = min.m_east_m + i * ydelta;
    gly[i] = max.m_east_m;
    
    // loop through edges and insert into grid structure 
//    CPolygon::VPointsIterator vtx0, vtx1;
//    CPolygon::Point *vtxa, *vtxb;
    CPosition *vtxa, *vtxb;
    Cell *p_gc;
    double xdiff, ydiff, tmax, xdir, ydir, t_near, tx, ty, inv_x, inv_y;
    // For distinct locations, there must be a difference in either the north or east directions,
    // so tx and ty can't both be set to dHUGE leading into iterative loop.  The variables tgcx and
    // tgcy used are set in all branches for all other cases, so the conditional branch selected
    // will never try and use the uninitialized tgcx/tgcy.  Both values are initialized to zero
    // to dismiss a compiler warning regarding use of potentially uninitialized variables.
    double tgcx(0), tgcy(0);
    double vx0, vy0, vx1, vy1;
    int gcx, gcy, sign_x, y_flag;
//    vtx0 = (Polygon_Points.end() - 1 );
    
//    for ( vtx1 = Polygon_Points.begin() ; vtx1 < Polygon_Points.end() ; vtx1++ ) {
    for (size_t iVertexIndex1=0,iVertexIndex0=(viVerticies.size()-1);iVertexIndex1<viVerticies.size();iVertexIndex1++ ) 
    {
        
        if (vposVertexContainer[viVerticies[iVertexIndex0]].m_east_m < vposVertexContainer[viVerticies[iVertexIndex1]].m_east_m) 
        {
            vtxa = &vposVertexContainer[viVerticies[iVertexIndex0]] ;
            vtxb = &vposVertexContainer[viVerticies[iVertexIndex1]] ;
        } else {
            vtxa = &vposVertexContainer[viVerticies[iVertexIndex1]] ;
            vtxb = &vposVertexContainer[viVerticies[iVertexIndex0]] ;
        }
        
        // Set x variable for the direction of the ray 
        xdiff = vtxb->m_north_m - vtxa->m_north_m ;
        ydiff = vtxb->m_east_m - vtxa->m_east_m;
        tmax = sqrt( xdiff * xdiff + ydiff * ydiff ) ;
        
        // if edge is of 0 length, ignore it (useless edge) 
        if  ( tmax != 0.0 ) {
            
            xdir = xdiff / tmax ;
            ydir = ydiff / tmax ;
            
            gcx = (int)(( vtxa->m_north_m - min.m_north_m ) * inv_xdelta) ;
            gcy = (int)(( vtxa->m_east_m - min.m_east_m ) * inv_ydelta) ;
            
            // get information about slopes of edge, etc 
            if ( vtxa->m_north_m == vtxb->m_north_m ) {
                sign_x = 0 ;
                tx = dHUGE ;
            } else {
                inv_x = tmax / xdiff ;
                tx = xdelta * (double)gcx + min.m_north_m - vtxa->m_north_m ;
                if ( vtxa->m_north_m < vtxb->m_north_m ) {
                    sign_x = 1 ;
                    tx += xdelta ;
                    tgcx = xdelta * inv_x ;
                } else {
                    sign_x = -1 ;
                    tgcx = -xdelta * inv_x ;
                }
                tx *= inv_x ;
            }
            
            if ( vtxa->m_east_m == vtxb->m_east_m ) {
                ty = dHUGE ;
            } else {
                inv_y = tmax / ydiff ;
                ty = (ydelta * (double)(gcy+1) + min.m_east_m - vtxa->m_east_m)* inv_y ;
                tgcy = ydelta * inv_y ;
            }
#if DEBUG_FLAG==1
            if ( gcy*XGridResolution+gcx > 24 )
                cout << "ERROR!\n";
#endif
            p_gc = &(VGrid.at(gcy*XGridResolution+gcx));
            
            vx0 = vtxa->m_north_m ;
            vy0 = vtxa->m_east_m ;
            
            t_near = 0.0 ;
            do {
                
                if ( tx <= ty ) {
                    gcx += sign_x ;
                    
                    ty -= tx ;
                    t_near += tx ;
                    tx = tgcx ;
                    
                    // note which edge is hit when leaving this cell 
                    if ( t_near < tmax ) {
                        if ( sign_x > 0 ) {
                            p_gc->gc_flags |= GC_R_EDGE_HIT ;
                            vx1 = glx[gcx] ;
                        } else {
                            p_gc->gc_flags |= GC_L_EDGE_HIT ;
                            vx1 = glx[gcx+1] ;
                        }
                        
                        // get new location 
                        vy1 = t_near * ydir + vtxa->m_east_m ;
                    } else {
                        // end of edge, so get exact value 
                        vx1 = vtxb->m_north_m ;
                        vy1 = vtxb->m_east_m ;
                    }
                    
                    y_flag = false ;
                    
                } else {
                    
                    gcy++ ;
                    
                    tx -= ty ;
                    t_near += ty ;
                    ty = tgcy ;
                    
                    // note top edge is hit when leaving this cell 
                    if ( t_near < tmax ) {
                        p_gc->gc_flags |= GC_T_EDGE_HIT ;
                        // this toggles the parity bit 
                        p_gc->gc_flags ^= GC_T_EDGE_PARITY ;
                        
                        // get new location 
                        vx1 = t_near * xdir + vtxa->m_north_m ;
                        vy1 = gly[gcy] ;
                    } else {
                        // end of edge, so get exact value 
                        vx1 = vtxb->m_north_m ;
                        vy1 = vtxb->m_east_m ;
                    }
                    
                    y_flag = true ;
                }
#if DEBUG_FLAG==1
                cout << " b4 setup grid data \n";
#endif
                // check for corner crossing, then mark the cell we're in 
                if (! Setup_GridData(*p_gc, vx0, vy0, vx1, vy1, *vtxa, *vtxb)) {
                    // warning, danger - we have just crossed a corner.
                    // There are all kinds of topological messiness we could
                    // do to get around this case, but they're a headache.
                    // The simplest recovery is just to change the extents a bit
                    // and redo the meshing, so that hopefully no edges will
                    // perfectly cross a corner.  Since it's a preprocess, we
                    // don't care too much about the time to do it.
                    //
                    
                    //                    cout << "WARNING CORNER PROBLEMS - redo structure!\n";
                    //                    sstrErrorMessage << "WARNING CORNER PROBLEMS - redo structure!\n";
                    // clean out all grid records 
                    
                    if ( VGrid.size()) {                        // skip if size = 0 - nothing to delete

                        // Delete Cell structures 
                        
                        for (auto it = VGrid.begin(); it != VGrid.end() ; ++it) {
#if DEBUG_FLAG==1
                            cout << "deleting Grid Cell Data\n";
#endif
                            it->Data.clear();
                        }
                        
                    }
                    
                    /* make the bounding box ever so slightly larger, hopefully
                    * changing the alignment of the corners.
                    */
                    xdiff = max.m_north_m - min.m_north_m;
                    ydiff = max.m_east_m - min.m_east_m;
                    double EPSILON = .00001;
                    if (min.m_north_m < ( min.m_north_m - (EPSILON * xdiff * 0.24) ) ) 
                        sstrErrorMessage << "rescale X ERRORR!! EROOR!!! \n";
                    if (min.m_east_m < ( min.m_east_m - (EPSILON * ydiff * 0.10 ) ) )
                        sstrErrorMessage << "rescale Y ERROR!! ERROR!! \n";
                    min.m_north_m -= EPSILON * xdiff * 0.24 ;
                    min.m_east_m -= EPSILON * ydiff * 0.10 ;
                    
                    /* yes, it's the dreaded goto - run in fear for your lives! */
                    attempt++;
#if DEBUG_FLAG==1
                    if (attempt > 5 ) {
                        sstrErrorMessage << "WARNING!!!  loooping at tryagain!\n";
                        //return;
                    }
#endif
                    goto TryAgain ; 
                }
                if ( t_near < tmax ) {
                    // note how we're entering the next cell
                    // TBD: could be done faster by incrementing index in the
                    // incrementing code, above 
#if DEBUG_FLAG==1
                    if((gcy*XGridResolution+gcx) >  24) {
                        sstrErrorMessage << "WARNING!!!\n";
                    }
#endif
                    p_gc = &(VGrid.at(gcy*XGridResolution+gcx) );
                    
                    if ( y_flag ) {
                        p_gc->gc_flags |= GC_B_EDGE_HIT ;
                        // this toggles the parity bit 
                        p_gc->gc_flags ^= GC_B_EDGE_PARITY ;
                    } else {
                        p_gc->gc_flags |=
                            ( sign_x > 0 ) ? GC_L_EDGE_HIT : GC_R_EDGE_HIT ;
                    }
                }
                
                vx0 = vx1 ;
                vy0 = vy1 ;
            }
            
            // have we gone further than the end of the edge? 
            while ( t_near < tmax ) ;
            
        }
        iVertexIndex0 = iVertexIndex1 ;
    } 
    vtxa = nullptr;    //we don't own this
    vtxb = nullptr;    //we don't own this

    // grid is all setup, now set up the inside/outside value of each corner.
    Setup_Corner_Value();
    
}

void CGrid::Setup_Corner_Value()  {
#if DEBUG_FLAG==1
    if (METHOD_DEBUG)
        cout << "Cgrid Setup_Corner_Value() Method\n";
#endif
    
    // Grid all set up, now set up the inside/outside value of each corner.
    auto it = VGrid.begin();
    auto it_next = VGrid.begin() + XGridResolution;
    
    int io_state;
    
    // we know the bottom and top rows are all outside, so no flag is set 
    for (int i = 1; i < YGridResolution ; i++) {
        
        // start outside 
        io_state = 0x0;   // reset each y turn 
        
        for (int j = 0 ; j < XGridResolution ; j++ ) {
            
            if (io_state) {
                // change cell left corners to inside
                it->gc_flags |= GC_TL_IN;
                it_next->gc_flags |= GC_BL_IN;
            }
            
            if (it->gc_flags & GC_T_EDGE_PARITY)
                io_state = !io_state;
            
            if (io_state) {
                // change cell right corners to inside 
                it->gc_flags |= GC_TR_IN;
                it_next->gc_flags |= GC_BR_IN;
            }
            
            ++it;
            ++it_next;
        }
    } 
    for (auto it = VGrid.begin(); it != VGrid.end(); ++it) {
        // reverse parity of edge clear - ( now 1 means edge clear ) 
        unsigned short gc_clear_flags = it->gc_flags ^ GC_ALL_EDGE_CLEAR;
        
        if (gc_clear_flags & GC_L_EDGE_CLEAR) {
            it->gc_flags |= GC_AIM_L;
        } else if (gc_clear_flags & GC_B_EDGE_CLEAR) {
            it->gc_flags |= GC_AIM_B;
        } else if ( gc_clear_flags & GC_R_EDGE_CLEAR) {
            it->gc_flags |= GC_AIM_R;
        } else if ( gc_clear_flags & GC_T_EDGE_CLEAR) {
            it->gc_flags |= GC_AIM_T;
        } else {
            // all edges are intersected on them, do full test 
            it->gc_flags |= GC_AIM_C;
        }
    } 
#if DEBUG_FLAG==1
    Print();
#endif
}

void CGrid::Print() {
    if (METHOD_DEBUG)
        cout << "Cgrid Print() Method\n";

    int i = 0;
    
    for (auto it = VGrid.begin() ; it != VGrid.end(); ++it) {
        cout << "\n Grid Cell " << i++ << "\n";
        printf("\nTOTAL EDGES:    %d    GC_FLAGS:    %u\n", it->tot_edges, it->gc_flags );
        
        for (auto it_cell = it->Data.begin(); it_cell != it->Data.end(); ++it_cell) {
            cout << "    xa,ya: " << it_cell->xa << "," << it_cell->ya << " ax, ay: " << it_cell->ax << "," << it_cell->ay << " slope " << it_cell->slope << "\n";
            cout << "    minx: " << it_cell->minx << " maxx: " << it_cell->maxx << " miny: " << it_cell->miny << " maxy: " << it_cell->maxy << "\n";
            cout << "    Vertex: (" << it_cell->VertexA.m_north_m <<" , " << it_cell->VertexA.m_east_m <<") , ( " << it_cell->VertexB.m_north_m << ", " << it_cell->VertexB.m_east_m << " ) \n";
        }
    }
}

bool CGrid::Setup_GridData(Cell &Cur_Cell, double xa, double ya, double xb, double yb, CPosition &vtxa, CPosition &vtxb) {
#if DEBUG_FLAG==1
    cout << "Cgrid Setup_GRidData() Method\n";
#endif
    
    double slope, inv_slope;
    
    if ( Near(ya, yb, NearEpsilon) ) {
        if ( Near(xa, xb, NearEpsilon) ) {
            /* edge is 0 length, so get rid of it */
            return false;
        } else {
            /* horizontal line */
            slope = dHUGE ;
            inv_slope = 0.0 ;
        }
    } else {
        if ( Near(xa, xb, NearEpsilon) ) {
            /* vertical line */
            slope = 0.0 ;
            inv_slope = dHUGE ;
        } else {
            slope = (xb-xa)/(yb-ya) ;
            inv_slope = (yb-ya)/(xb-xa) ;
        }
    }
    
    Cur_Cell.tot_edges++;
    Cur_Cell.Data.resize((Cur_Cell.tot_edges <= 1) ? 1 : Cur_Cell.tot_edges);
    
    auto it_cell = Cur_Cell.Data.end() - 1;
    
    it_cell->slope = slope ;
    it_cell->inv_slope = inv_slope ;
    
    it_cell->xa = xa ;
    it_cell->ya = ya ;
    
    if ( xa <= xb ) {
        it_cell->minx = xa ;
        it_cell->maxx = xb ;
    } else {
        it_cell->minx = xb ;
        it_cell->maxx = xa ;
    }
    if ( ya <= yb ) {
        it_cell->miny = ya ;
        it_cell->maxy = yb ;
    } else {
        it_cell->miny = yb ;
        it_cell->maxy = ya ;
    }
    
    /* P2 - P1 */
    it_cell->ax = xb - xa ;
    it_cell->ay = yb - ya ;
    
    it_cell->VertexA = vtxa;
    it_cell->VertexB = vtxb;
    
    return true;
}


bool CGrid::InPolygon(double x, double y, const CPosition &min, stringstream &sstrErrorMessage) {
    
#if DEBUG_FLAG==1
    cout << "Cgrid InPolygon() Method\n";
#endif
    
    bool inside_flag = false;
    Cell *p_gc = nullptr;
    double bx = 0, by = 0, cx = 0, cy= 0, alpha=0, beta=0, cornerx=0, cornery=0, denom=0;    
    
    // What cell are we in?
    double ycell = ( y - min.m_east_m ) * inv_ydelta;
    double xcell = ( x - min.m_north_m ) * inv_xdelta;
    if (((int) ycell) * XGridResolution + (int)xcell > 24 ) {
        cout << "ERROR!!! \n";
    }
    p_gc = &(VGrid.at( ((int) ycell) * XGridResolution + (int)xcell ));
    
    int count = p_gc->tot_edges;
    
    // is cell simple?
    if (! count) {
        
        // simple cell, so if left lower corner is in, then cell is inside.
        inside_flag =  ( p_gc->gc_flags & GC_BL_IN ) ? 1 : 0 ;
#if DEBUG_FLAG==1
        cout << " Found Simple Cell \n";
#endif
    } else {
        
        /* no, so find an edge which is free. */
        unsigned short gc_flags = p_gc->gc_flags ;
        
        switch (gc_flags & GC_AIM ) {
            // CASE 1:   left edge is clear, shoot X- ray 
            //    GC_AIM_L
        case (0<<10):                // aim towards bottom edge
            //left edge is clear, shoot X-ray
            inside_flag = (gc_flags & GC_BL_IN)? 1 : 0 ;
            
            for (auto it_cell = p_gc->Data.begin(); it_cell != p_gc->Data.end(); ++it_cell) {
                
                // test if y is between edges 
                if ( y >= it_cell->miny && y < it_cell->maxy ) {
                    
                    if ( x > it_cell->maxx ) {
                        
                        inside_flag = !inside_flag ;
                        
#if DEBUG_FLAG==1
                        cout << " case 1 (a) - x > maxx test \n" ;
#endif
                        
                    } else if ( x > it_cell->minx ) {
                        
                        // full computation 
                        if ( ( it_cell->xa -    ( it_cell->ya - y ) * it_cell->slope ) < x )  {
                            inside_flag = !inside_flag ;
                            
#if DEBUG_FLAG==1
                            cout << " case 1 (b) - full computation \n";
#endif
                        }
                    }
                }
            }
            break;                // end left edge clear case
            
            // CASE 2: bottom edge is clear, shoot Y+ ray 
        case (1<<10):
            
            // note - this next statement requires that GC_BL_IN is 1 
            inside_flag = (gc_flags & GC_BL_IN ) ? 1 : 0 ;
            
            for (auto it_cell = p_gc->Data.begin(); it_cell != p_gc->Data.end(); ++it_cell) {
                
                // test if x is between edges 
                if ( x >= it_cell->minx && x < it_cell->maxx ) {
                    
                    if ( y > it_cell->maxy ) {
                        
                        inside_flag = !inside_flag ;
#if DEBUG_FLAG==1
                        cout << "case 2 (a) - y > maxy test \n";
#endif
                        
                    } else if ( y > it_cell->miny ) {
                        // full computation 
                        
                        if ( ( it_cell->ya - ( it_cell->xa - x ) * it_cell->inv_slope ) < y ) {
                            inside_flag = !inside_flag ;
                            
#if DEBUG_FLAG==1
                            cout << "case 2 (b) - full computation \n";
#endif
                        }
                    }
                }
            }
            break;            // end bottom edge clear case
            
            // CASE 3:    right edge is clear, shoot X+ ray 
        case (2<<10):
            inside_flag = (gc_flags & GC_TR_IN) ? 1 : 0 ;
            // TBD: Note, we could have sorted the edges to be tested
            // by miny or somesuch, and so be able to cut testing
            // short when the list's miny > point.y .
            
            for (auto it_cell = p_gc->Data.begin(); it_cell != p_gc->Data.end(); ++it_cell) {
                
                // test if y is between edges 
                if ( y >= it_cell->miny && y < it_cell->maxy ) {
                    
                    if ( x <= it_cell->minx ) {
                        
                        inside_flag = !inside_flag ;
                        
#if DEBUG_FLAG==1
                        cout << "case 3 (a) - y > maxy test \n";
#endif
                        
                    } else if ( x <= it_cell->maxx ) {
                        
                        // full computation 
                        if ( ( it_cell->xa - ( it_cell->ya - y ) * it_cell->slope ) >= x ) {
                            inside_flag = !inside_flag ;
#if DEBUG_FLAG==1
                            cout << "case 3 (b) - full computation \n";
#endif
                        }
                    }
                }
            }
            break;        // end right edge clear case
            
            // CASE 4:    top edge is clear, shoot Y+ ray 
        case (3<<10):
            inside_flag = (gc_flags & GC_TR_IN) ? 1 : 0 ;
            for (auto it_cell = p_gc->Data.begin(); it_cell != p_gc->Data.end(); ++it_cell) {
                // test if x is between edges 
                if ( x >= it_cell->minx && x < it_cell->maxx ) {
                    if ( y <= it_cell->miny ) {
                        inside_flag = !inside_flag ;
                        
#if DEBUG_FLAG==1
                        cout << "case 4 (a) - y <= miny test \n";
#endif
                    } else if ( y <= it_cell->maxy ) {
                        // full computation 
                        if ( ( it_cell->ya - ( it_cell->xa - x ) * it_cell->inv_slope ) >= y ) {
                            inside_flag = !inside_flag ;
#if DEBUG_FLAG==1
                            cout << "case 4 (b) - full computation \n";
#endif
                        }
                    }
                }
            }
            break;
            
            // CASE 5:    no edge is clear, test against the bottom left corner.
        case (4<<10):
            //    We use Franklin Antonio's algorithm (Graphics Gems III).
            
            //    TBD: Faster yet might be to test against the closest
            //    corner to the cell location, but our hope is that we
            //    rarely need to do this testing at all.
            inside_flag = ((gc_flags & GC_BL_IN) == GC_BL_IN) ;
            
            /// get lower left corner coordinate 
            cornerx = glx[ (int)xcell ];
            cornery = gly[ (int)ycell ];
            
            for (auto it_cell = p_gc->Data.begin(); it_cell != p_gc->Data.end(); ++it_cell) {
                
                // quick out test: if test point is
                // less than minx & miny, edge cannot overlap.
                
                if ( x >= it_cell->minx && y >= it_cell->miny ) {
                    
                    // quick test failed, now check if test point and
                    // corner are on different sides of edge.
                    
                    if ( ! bx ) {
                        // Compute these at most once for test: P3 - P4 
                        bx = x - cornerx ;
                        by = y - cornery ;
                    }
                    
                    denom = it_cell->ay * bx - it_cell->ax * by ;
                    
                    if ( denom != 0.0 ) {
                        
                        // lines are not collinear, so continue: P1 - P3 
                        cx = it_cell->xa - x ;
                        cy = it_cell->ya - y ;
                        alpha = by * cx - bx * cy ;
                        beta = it_cell->ax * cy - it_cell->ay * cx ;
                        
                        if ( ( denom > 0.0 ) && 
                            (  ! ( alpha < 0.0 || alpha >= denom )             // test edge hit 
                            &&  ! ( beta < 0.0 || beta >= denom ) ) ) {        // polygon edge hit 
                            
                            inside_flag = !inside_flag;
#if DEBUG_FLAG==1
                            cout << " case 5 (a) \n";
#endif
                        }
                        if ( ( denom < 0.0 ) &&
                            (  ! ( alpha > 0.0 || alpha <= denom )            // test edge hit 
                            && ! ( beta > 0.0 || beta <= denom ) ) ) {        // polygon edge hit 
                            
                            inside_flag = !inside_flag ;
#if DEBUG_FLAG==1
                            cout << " case 5 (b) \n";
#endif
                        }
                    }
                    
                }
                
            }
            break;
            
        default:
            sstrErrorMessage << "WARNING! should never end up HERE! \n" ;
            sstrErrorMessage << "HERE is in the default of case statement of method InPolygon of class CGrid\n";
            break;
        }
        //        cout << "after main test " << endl;
        // double check not on vertex or line if false
        // better to save points in structure and not test whole thing 
        // but do this later
        if ((!inside_flag) && (count) ){
            CPosition *vertexA = nullptr, *vertexB = nullptr;
            CPosition posX(x,y);
            
            for (auto it_cell = p_gc->Data.begin(); it_cell != p_gc->Data.end(); ++it_cell) {
                    //def is_on(a, b, c):
                    //    "Return true iff point c intersects the line segment from a to b."
                    //    # (or the degenerate case that all 3 points are coincident)
                    //    return (collinear(a, b, c)
                    //            and (within(a.x, c.x, b.x) if a.x != b.x else
                    //                 within(a.y, c.y, b.y)))
                    //
                    //def collinear(a, b, c):
                    //    "Return true iff a, b, and c all lie on the same line."
                    //    return (b.x - a.x) * (c.y - a.y) == (c.x - a.x) * (b.y - a.y)
                    //
                    //def within(p, q, r):
                    //    "Return true iff q is between p and r (inclusive)."
                    //    return p <= q <= r or r <= q <= p

                // OR
                //
                    //def distance(a,b):
                    //    return sqrt((a.x - b.x)**2 + (a.y - b.y)**2)
                    //
                    //def is_between(a,c,b):
                    //    return distance(a,c) + distance(c,b) == distance(a,b)

                vertexA = &it_cell->VertexA;
                vertexB = &it_cell->VertexB;
                //    cout << " Points tested are: (" << vertexA->X << ", " << vertexA->Y << ")  & ( " << vertexB->X << ", " << vertexB->Y << ") \n";
                double dDistanceAtoB = vertexA->relativeDistance2D_m(*vertexB);
                double dDistanceAtoX = vertexA->relativeDistance2D_m(posX);
                double dDistanceBtoX = vertexB->relativeDistance2D_m(posX);
                double dResult = dDistanceAtoX + dDistanceBtoX - dDistanceAtoB;

                if (fabs(dResult) < 1.0e-2 )
                {
                    inside_flag = 1;
                    break;
                }
            } 
        } 
    }
    return inside_flag;
}

};      //namespace n_FrameworkLib
              
