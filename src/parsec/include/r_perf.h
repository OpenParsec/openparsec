/*
 * PARSEC HEADER: r_perf.h
 */

#ifndef _R_PERF_H_
#define _R_PERF_H_


// ----------------------------------------------------------------------------
// RENDERING SUBSYSTEM (R) PERF group                                         -
// ----------------------------------------------------------------------------


void	R_ResetGeometryPerfStats();
void	R_QueryGeometryPerfStats( geomperfstats_s *perfstats );
void	R_ResetRasterizationPerfStats();
void	R_QueryRasterizationPerfStats( rastperfstats_s *perfstats );


#endif // _R_PERF_H_


