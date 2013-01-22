/*
 * PARSEC HEADER: xac_mat2.h
 */

#ifndef _XAC_MAT2_H_
#define _XAC_MAT2_H_


// xac_mat2.c implements the following functions
// ---------------------------------------------
//	geomv_t	VctLen( Vector3 *vec );
//	geomv_t	VctLenX( Vector3 *vec );
//	void	NormVct( Vertex3 *vec );
//	void	NormVctX( Vertex3 *vec );
//	void	NormMtx( Xmatrx matrix );
//	void	MakeIdMatrx( Xmatrx matrx );
//	void	MakeNonTranslationMatrx( const Xmatrx matrx, Xmatrx dmatrx );
//	void	CalcOrthoInverse( const Xmatrx matrx, Xmatrx dmatrx );
//	void	CalcObjSpaceCamera( const GenObject *objectp, Vertex3 *cameravec );
//	void	TransformVolume( const Xmatrx matrx, Plane3 *vol_in, Plane3 *vol_out, dword cullmask );
//	void	BackTransformVolume( const Xmatrx matrx, Plane3 *vol_in, Plane3 *vol_out, dword cullmask );
//	int		QuaternionIsUnit( const Quaternion *quat );
//	int		QuaternionIsUnit_f( const Quaternion_f *quat );
//	void	QuaternionMakeUnit( Quaternion *quat );
//	void	QuaternionMakeUnit_f( Quaternion_f *quat );
//	void	QuaternionInvertUnit( Quaternion *quat );
//	void	QuaternionInvertUnit_f( Quaternion_f *quat );
//	void	QuaternionInvertGeneral( Quaternion *quat );
//	void	QuaternionInvertGeneral_f( Quaternion_f *quat );
//	void	QuaternionFromMatrx( Quaternion *quat, const Xmatrx matrix );
//	void	QuaternionFromMatrx_f( Quaternion_f *quat, const Xmatrx matrix );
//	void	MatrxFromQuaternion( Xmatrx matrix, const Quaternion *quat );
//	void	MatrxFromQuaternion_f( Xmatrx matrix, const Quaternion_f *quat );
//	void	MatrxFromAngularDisplacement( Xmatrx matrix, bams_t angle, Vertex3 *axis );
//	void	QuaternionMUL( Quaternion *qd, const Quaternion *qb, const Quaternion *qa );
//	void	QuaternionMUL_f( Quaternion_f *qd, const Quaternion_f *qb, const Quaternion_f *qa );
//	void	QuaternionSlerp( Quaternion *qd, const Quaternion *qa, const Quaternion *qb, float alpha );
//	void	QuaternionSlerp_f( Quaternion_f *qd, const Quaternion_f *qa, const Quaternion_f *qb, float alpha );
//  void	Hermite_Interpolate( Vector3* dest, float t, const Vector3* start, const Vector3* end, const Vector3* start_tan, const Vector3* end_tan, float start_tan_scale, float end_tan_scale );
//  void    DumpMatrix( Xmatrx mat );

// xac_mat2.c implements the following variables
// ---------------------------------------------
//	extern pXmatrx DestXmatrx;


#endif // _XAC_MAT2_H_


