#include "oxsphericalharmonics.h"
#include <math.h>
#include <sstream>

namespace OX {
	namespace {
		double clamp( double v, double min, double max ) {
			return ( v < min ? min : ( v > max ? max : v ) );
		};
	}

	namespace SphericalHarmonics {

		// ���ʒ��a�֐��Q���쐬
		std::vector< std::function< double( double th, double phi )> > createSphericalHarmonicsFuncs( uint32_t level ) {
			uint32_t fnum = ( level + 1 ) * ( level + 1 );
			std::shared_ptr< std::vector< std::function< double( double th, double phi ) > > >
				ylist( new std::vector< std::function< double( double th, double phi ) > >( fnum ) );
			std::shared_ptr< std::vector< std::function< double( double x ) > > >
				plist( new std::vector< std::function< double( double x ) > >( fnum ) );

			// ���W�����h���������������_���Q���쐬
			// P_m_m�Q�쐬
			( *plist )[ 0 ] = []( double x ) {
				return 1.0;
			};
			double evenFact = 1.0;	// n!!
			for ( uint32_t m = 1; m <= level; ++m ) {
				evenFact *= 2.0 * m - 1.0f;
				int sign = ( m % 2 ? -1 : 1 );
				auto p_m_m = [ m, sign, evenFact ]( double x ) {
					return sign * evenFact * pow( sqrt( 1.0 - x * x ), m );
				};
				int idx = Parameter::toIdx( m, m );
				( *plist )[ idx ] = p_m_m;
			}

			// P_mp1_m�Q�쐬
			for ( uint32_t m = 0; m < level; ++m ) {
				int m_m_idx = Parameter::toIdx( m, m );
				auto p_mp1_m = [ m, plist, m_m_idx ]( double x ) {
					return x * ( 2.0 * m + 1 ) * ( *plist )[ m_m_idx ]( x );
				};
				int idx = Parameter::toIdx( m + 1, m );
				( *plist )[ idx ] = p_mp1_m;
			}

			// P_mpi_m�Q�쐬
			for ( int32_t m = 0; m < (int32_t)level - 1; ++m ) {
				// �Q�����ň��Â鎮�Ɏ����쐬
				uint32_t lidx = m + 2;
				for ( uint32_t l = lidx; l <= level; ++l ) {
					int mn2_m_idx = Parameter::toIdx( l - 2, m );
					int mn1_m_idx = Parameter::toIdx( l - 1, m );
					auto p_l_m = [ l, m, plist, mn2_m_idx, mn1_m_idx ]( double x ) {
						return ( x * ( 2.0 * l - 1 ) * ( *plist )[ mn1_m_idx ]( x ) - ( l + m - 1 ) * ( *plist )[ mn2_m_idx ]( x ) ) / ( l - m );
					};
					int idx = Parameter::toIdx( l, m );
					( *plist )[ idx ] = p_l_m;
				}
			}

			// P_l_nm�Q���쐬
			// P_l_pm�Q�Ɠ����ɂȂ�
			for ( uint32_t l = 1; l <= level; ++l ) {
				for ( int32_t m = 1; m <= (int32_t)l; ++m ) {
					int nidx = Parameter::toIdx( l, -m );
					int pidx = Parameter::toIdx( l, m );
					( *plist )[ nidx ] = ( *plist )[ pidx ];
				}
			}

			// �K�惊�X�g�쐬
			std::vector< double > facts( level * 2 + 1 );
			double fact = 1.0;
			facts[ 0 ] = fact;
			for ( size_t i = 1; i < facts.size(); ++i ) {
				facts[ i ] = facts[ i - 1 ] * i;
			}

			// ���ʒ��a�֐�y_l_0�Q���쐬
			const double _2pi = 2.0 * 3.14159265358979323846;
			for ( uint32_t l = 0; l <= level; ++l ) {
				int idx = Parameter::toIdx( l, 0 );
				double c_l_0 = sqrt( ( 2 * l + 1 ) / ( 2.0 * _2pi ) );
				( *ylist )[ idx ] = [ c_l_0, plist, idx ]( double th, double phi ) {
					return c_l_0 * ( *plist )[ idx ]( cos( th ) );
				};
			}

			// ���ʒ��a�֐�y_l_pm, y_l_nm,�Q���쐬
			for ( int32_t m = 1; m <= (int32_t)level; ++m ) {
				for ( uint32_t l = m; l <= level; ++l ) {
					int pre_m_idx = Parameter::toIdx( l, m - 1 );
					int pidx = Parameter::toIdx( l, m );
					double f = facts[ l - m ] / facts[ l + m ];
					double c_l_pm = sqrt( f * ( 2 * l + 1 ) / _2pi );
					// y_l_pm
					( *ylist )[ pidx ] = [ m, c_l_pm, plist, pidx ]( double th, double phi ) {
						return c_l_pm * ( *plist )[ pidx ]( cos( th ) ) * cos( m * phi );
					};
					// y_l_nm
					int nidx = Parameter::toIdx( l, -m );
					( *ylist )[ nidx ] = [ m, c_l_pm, plist, nidx ]( double th, double phi ) {
						double sin_th  = sin( th );
						double sin_phi = sin( m * phi );
						double p = ( *plist )[ nidx ]( cos( th ) );
						return c_l_pm * ( *plist )[ nidx ]( cos( th ) ) * sin( m * phi );
					};
				}
			}

			return *ylist;
		}

		// ����p�����[�^����L���[�u�}�b�v�쐬
		std::vector< ImageBlock > createCubeMapFromParameters( const Result &res, uint32_t width, CubeMapType mapType, const std::function< void( uint64_t count, uint64_t procCount ) > &proc ) {
			uint32_t maxLevel = res.getMaxLevel();
			const auto &paramR = res.getParamList( SphericalHarmonics::ColorType::ColorType_R );
			const auto &paramG = res.getParamList( SphericalHarmonics::ColorType::ColorType_G );
			const auto &paramB = res.getParamList( SphericalHarmonics::ColorType::ColorType_B );
			std::vector< std::vector< Parameter > > params;
			params.push_back( paramR );
			params.push_back( paramG );
			params.push_back( paramB );
			auto yfuncs = createSphericalHarmonicsFuncs( res.getMaxLevel() );

			ImageBlockCustom images[] = {
				ImageBlockCustom( width, width, 3, 0 ),
				ImageBlockCustom( width, width, 3, 0 ),
				ImageBlockCustom( width, width, 3, 0 ),
				ImageBlockCustom( width, width, 3, 0 ),
				ImageBlockCustom( width, width, 3, 0 ),
				ImageBlockCustom( width, width, 3, 0 ),
			};

			uint64_t count = 0;
			uint64_t procCount = width * width * CubeData::Face::Face_Num;
			for ( uint32_t f = 0; f < CubeData::Face::Face_Num; ++f ) {
				CubeData::Face face = ( CubeData::Face )f;
				uint8_t *p = images[ f ].p();
				for ( uint32_t tv = 0; tv < width; ++tv ) {
					for ( uint32_t tu = 0; tu < width; ++tu ) {
						double th, phi;
						double l = CubeData::getPolar( face, width, tu, tv, th, phi );

						// (tu, tv)�ɑΉ�����F���Z�o
						double r = 0.0, g = 0.0, b = 0.0;
						for ( uint32_t y = 0; y < yfuncs.size(); ++y ) {
							double yval = yfuncs[ y ]( th, phi );
							r += paramR[ y ].value() * yval;
							g += paramG[ y ].value() * yval;
							b += paramB[ y ].value() * yval;
						}
						p[ 0 ] = (uint8_t)( clamp( r, 0.0, 1.0 ) * 255 );
						p[ 1 ] = (uint8_t)( clamp( g, 0.0, 1.0 ) * 255 );
						p[ 2 ] = (uint8_t)( clamp( b, 0.0, 1.0 ) * 255 );
						p += 3;
						proc( count, procCount );
						count++;
					}
				}
			}

			std::vector< ImageBlock > outImageBlocks;

			if ( mapType == CubeMapType::Separable ) {
				for ( int i = 0; i < 6; ++i ) {
					outImageBlocks.push_back( images[ i ] );
				}
			}
			 
			else if ( mapType == CubeMapType::Horizontal_Cross ) {
				// ���N���X�ɂ܂Ƃ߂�
				uint32_t bpc = 3;	// byteParColor
				ImageBlockCustom hznImage( width * 4, width * 3, bpc, 0 );
				uint32_t pitchByte = 4 * width * bpc;
				uint32_t offsetsByte[ 6 ] = {
					pitchByte * width + 2 * bpc * width,	// PX
					pitchByte * width,						// NX
					bpc * width,							// PY
					pitchByte * 2 * width + width * bpc,	// NY
					pitchByte * width + width * bpc,		// PZ
					pitchByte * width + 3 * width * bpc,	// NZ
				};
				uint8_t *ptr = hznImage.p();
				memset( ptr, 0x00, hznImage.size() );
				uint32_t lineByte = width * bpc;
				for ( uint32_t i = 0; i < 6; ++i ) {
					uint8_t *dest = ptr + offsetsByte[ i ];
					uint8_t *src = images[ i ].p();
					for ( uint32_t y = 0; y < width; ++y ) {
						memcpy( dest, src, lineByte );
						dest += pitchByte;
						src += lineByte;
					}
				}
				outImageBlocks.push_back( hznImage );
			}

			return outImageBlocks;
		}





		void CubeData::getXYZ( Face face, int32_t w, int32_t tu, int32_t tv, double &x, double &y, double &z ) {
			static std::vector< std::function< void( int32_t w, int32_t tu, int32_t tv, double *x, double *y, double *z ) > > funcs = {
				[]( int32_t w, int32_t tu, int32_t tv, double *x, double *y, double *z ) {
				// XP
				*x = 0.5 * w;
				*y = -( tv % w ) - 0.5 + 0.5 * w;
				*z = -( tu % w ) - 0.5 + 0.5 * w;
			},
				[]( int32_t w, int32_t tu, int32_t tv, double *x, double *y, double *z ) {
				// XM
				*x = -0.5 * w;
				*y = -( tv % w ) - 0.5 + 0.5 * w;
				*z = ( tu % w ) + 0.5 - 0.5 * w;
			},
				[]( int32_t w, int32_t tu, int32_t tv, double *x, double *y, double *z ) {
				// YP
				*x = ( tu % w ) + 0.5 - 0.5 * w;
				*y = 0.5 * w;
				*z = ( tv % w ) + 0.5 - 0.5 * w;
			},
				[]( int32_t w, int32_t tu, int32_t tv, double *x, double *y, double *z ) {
				// YM
				*x = ( tu % w ) + 0.5 - 0.5 * w;
				*y = -0.5 * w;
				*z = -( tv % w ) - 0.5 + 0.5 * w;
			},
				[]( int32_t w, int32_t tu, int32_t tv, double *x, double *y, double *z ) {
				// ZP
				*x = ( ( tu % w ) + 0.5 - 0.5 * w );
				*y = ( -( tv % w ) - 0.5 + 0.5 * w );
				*z = 0.5 * w;
			},
				[]( int32_t w, int32_t tu, int32_t tv, double *x, double *y, double *z ) {
				// ZM
				*x = -( tu % w ) - 0.5 + 0.5 * w;
				*y = -( tv % w ) - 0.5 + 0.5 * w;
				*z = -0.5 * w;
			}
			};
			funcs[ (int)face ]( w, tu, tv, &x, &y, &z );
			x /= 0.5 * w;
			y /= 0.5 * w;
			z /= 0.5 * w;
		}

		// �w���UV�ʒu�ɑ΂���ɍ��W���擾
		// �߂�l : �w��UV�܂ł̋���
		double CubeData::getPolar( Face face, int32_t w, int32_t tu, int32_t tv, double &th, double &phi ) {
			double x, y, z;
			getXYZ( face, w, tu, tv, x, y, z );
			double l = sqrt( x * x + y * y + z * z );
			x /= l;
			y /= l;
			z /= l;
			th = acos( y );
			phi = atan2( z, x );
			return l;
		}

		// �w���UV�ʒu�ɑ΂���XYZ���W���擾 (-1,-1,-1)�`(1,1,1)
		void CubeData::getXYZ( Face face, int32_t tu, int32_t tv, double &x, double &y, double &z ) const {
			const int32_t w = getTexelSize();
			getXYZ( face, w, tu, tv, x, y, z );
		}

		// �w���UV�ʒu�ɑ΂���ɍ��W���擾
		double CubeData::getPolar( Face face, int32_t tu, int32_t tv, double &th, double &phi ) const {
			return getPolar( face, getTexelSize(), tu, tv, th, phi );
		}




		// �C���f�b�N�X���p�����[�^�ɕύX
		void Parameter::toLM( uint32_t idx, uint32_t &l, int32_t &m ) {
			l = (uint32_t)sqrt( idx );
			m = idx - l * l - l;
		}

		// �p�����[�^���C���f�b�N�X�ɕύX
		uint32_t Parameter::toIdx( uint32_t l, int32_t m ) {
			return l * l + l + m;
		}

		// level�擾
		uint32_t Parameter::l() const {
			return l_;
		}

		// m�擾
		int32_t Parameter::m() const {
			return m_;
		}

		// �l�擾
		double Parameter::value() const {
			return value_;
		}

		// �L���ȃp�����[�^�H
		bool Parameter::isValid() const {
			return !( l_ == invalidParam_g );
		}




		// �����Ԃ��擾
		ResultState Result::getState() const {
			return state_;
		}

		// ���莞�̍ő�Level���擾
		uint32_t Result::getMaxLevel() const {
			return maxLevel_;
		}

		// �p�����[�^���X�g�擾
		const std::vector< Parameter > &Result::getParamList( ColorType ctype ) const {
			static std::vector< Parameter > nullParamVec;
			if ( (size_t)ctype >= paramsVec_.size() ) {
				return nullParamVec;
			}
			return paramsVec_[ (size_t)ctype ];
		}

		// �w��C���f�b�N�X�̃p�����[�^���擾
		Parameter Result::getParam( ColorType ctype, uint32_t l, int32_t m ) const {
			// �����p�����[�^��
			// (l,m) = (0,0), (1,-1), (1,0), (1,1), (2,-2), (2,-1), ...
			// �ƕ���ł���O��

			size_t colorIdx = (size_t)ctype;
			if ( colorIdx >= paramsVec_.size() || (int32_t)l < -m || (int32_t)l < m )
				return Parameter();

			int32_t idx = l * l + l + m;
			return ( idx >= paramsVec_[ colorIdx ].size() ? Parameter() : paramsVec_[ colorIdx ][ idx ] );
		}



		// ���莞��band order level�̍ő�l���擾
		uint32_t Estimater::getMaxLevel() const {
			return maxLevel_;
		}



		// ����
		//  �߂�l : �G���[�����������ꍇ�͗L�������񂪕Ԃ�
		Error SphereEstimater::estimate( const SphereData *sphere ) {
			if ( sphere == 0 )
				return Error( "Null object" );
			return Error();
		}



		// ����
		Error CubeEstimater::estimate( const CubeData *cube, Result &res, const std::function< void( uint64_t count, uint64_t procCount ) > &proc ) {
			if ( cube == 0 )
				return Error( "Null object" );

			// �}�b�v�̕ӂ̃e�N�Z����
			double texelSize2 = cube->getTexelSize();
			texelSize2 *= texelSize2;

			// (l,m)�ɑΉ��������ʒ��a�֐��ƃp�����[�^�z���p��
			auto shFuncs = createSphericalHarmonicsFuncs( maxLevel_ );
			std::vector< double > coefsR( shFuncs.size() );
			std::vector< double > coefsG( shFuncs.size() );
			std::vector< double > coefsB( shFuncs.size() );
			for ( size_t i = 0; i < shFuncs.size(); ++i ) {
				coefsR[ i ] = 0.0;
				coefsG[ i ] = 0.0;
				coefsB[ i ] = 0.0;
			}

			// 6�ʂ��ꂼ����C�e���[�V����
			int32_t width = cube->getTexelSize();
			uint32_t procCount = width * width * (size_t)CubeData::Face::Face_Num;
			uint32_t count = 0;
			for ( size_t i = 0; i < (size_t)CubeData::Face::Face_Num; ++i ) {
				CubeData::Face face = ( CubeData::Face )i;
				for ( int32_t v = 0; v < width; ++v ) {
					for ( int32_t u = 0; u < width; ++u ) {
						double th, phi;
						RGBA value = cube->getValue( face, u, v );
						double l = cube->getPolar( face, u, v, th, phi );

						// �ey_lm�֐��ɂ��Ēl�Z�o
						for ( size_t f = 0; f < shFuncs.size(); ++f ) {
							double shVal = shFuncs[ f ]( th, phi ) / ( l * l * l );
							coefsR[ f ] += value.dr() * shVal;
							coefsG[ f ] += value.dg() * shVal;
							coefsB[ f ] += value.db() * shVal;
						}
						proc( count, procCount );
						count++;
					}
				}
			}

			// �W���p�����[�^���i�[
			std::vector< Parameter > paramsR;
			std::vector< Parameter > paramsG;
			std::vector< Parameter > paramsB;
			for ( size_t i = 0; i < coefsR.size(); ++i ) {
				uint32_t l;
				int32_t m;
				Parameter::toLM( (uint32_t)i, l, m );
				paramsR.push_back( Parameter( l, m, coefsR[ i ] * 4.0 / texelSize2 ) );
				paramsG.push_back( Parameter( l, m, coefsG[ i ] * 4.0 / texelSize2 ) );
				paramsB.push_back( Parameter( l, m, coefsB[ i ] * 4.0 / texelSize2 ) );
			}

			std::vector< std::vector< Parameter > > paramsVec;
			paramsVec.push_back( paramsR );
			paramsVec.push_back( paramsG );
			paramsVec.push_back( paramsB );

			res = Result( maxLevel_, paramsVec );

			return Error();
		}



		// ������
		//  fileNames : 6�ʂ̃t�@�C����(�E�A���A�O�A��A��A���̏�)
		Error CubeDataFromImage::initialize( const std::vector< std::string > &fileNames ) {
			if ( fileNames.size() < 6 ) {
				return Error( "lack of cube map files." );
			}
			std::stringstream ss;
			for ( size_t i = 0; i < fileNames.size(); ++i ) {
				ImageBlock block = ImageUtil::createImageBlockFromFile( fileNames[ i ].c_str() );
				if ( block.isExist() == false ) {
					ss << "invalid file. [" << fileNames[ i ] << "]";
					return Error( ss.str() );
				}
				if (
					i != 0 &&
					( images_[ 0 ].width() != block.width() ||
					  images_[ 0 ].height() != block.height() )
				) {
					ss << "invalid file format or texture size. ["
						<< fileNames[ i ]
						<< " : width = " << block.width()
						<< ", height = " << block.height()
						<< "]";
					return Error( ss.str() );
				}
				if ( block.width() != block.height() ) {
					ss << "texture is not square. ["
						<< fileNames[ i ]
						<< " : width = " << block.width()
						<< ", height = " << block.height()
						<< "]";
					return Error( ss.str() );
				}
				images_[ i ] = block;
			}
			return Error();
		}

		// �w���UV�ʒu�ɑ΂���l���擾
		RGBA CubeDataFromImage::getValue( Face face, int32_t tu, int32_t tv ) const {
			const int32_t w = images_[ (int)face ].width();
			const int32_t u = tu % w;
			const int32_t v = tv % w;
			uint8_t bpc = images_[ (int)face ].bytePerColor();
			uint8_t *p = images_[ (int)face ].p() + bpc * ( w * v + u );
			uint8_t a = ( bpc == 3 ? 255 : p[ 3 ] );
			return RGBA( p[ 0 ], p [ 1 ], p[ 2 ], a );
		}

		// �}�b�v�̃e�N�Z���T�C�Y���擾
		uint32_t CubeDataFromImage::getTexelSize() const {
			return images_[ 0 ].width();
		}
	}
}