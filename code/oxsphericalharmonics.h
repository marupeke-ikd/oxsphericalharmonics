#ifndef __ox_oxsphericalharmonics_h__
#define __ox_oxsphericalharmonics_h__

// ���ʒ��a�֐�

#include <string>
#include <vector>
#include <functional>
#include "oximageutil.h"

namespace OX {
	namespace SphericalHarmonics {

		// RGBA
		struct RGBA {
			uint8_t r_ = 0;
			uint8_t g_ = 0;
			uint8_t b_ = 0;
			uint8_t a_ = 0;
			RGBA() {}
			RGBA( uint8_t r, uint8_t g, uint8_t b, uint8_t a ) : r_( r ), g_( g ), b_( b ), a_( a ) {}
			double dr() const { return r_ / 255.0; };
			double dg() const { return g_ / 255.0; };
			double db() const { return b_ / 255.0; };
			double da() const { return a_ / 255.0; };
		};

		// ���`�f�[�^
		class SphereData {
		public:
			SphereData() {}
			virtual ~SphereData() {}

			// �w��̊p�x�ɑ΂���l���擾
			//  th  : �ܓx�p��(0�`��)
			//  phi : �o�x�p��(0�`2��)
			virtual double getValue( double th, double phi ) = 0;
		};

		// �L���[�u�}�b�v�f�[�^
		class CubeData {
		public:
			enum Face {
				PX,	// X+
				NX,	// X-
				PY,	// Y+
				NY,	// Y-
				PZ,	// Z+
				NZ,	// Z-
				Face_Num
			};

			// �w���UV�ʒu�ɑ΂���XYZ���W���擾
			static void getXYZ( Face face, int32_t w, int32_t tu, int32_t tv, double &x, double &y, double &z );

			// �w���UV�ʒu�ɑ΂���ɍ��W���擾
			// �߂�l : �w��UV�܂ł̋���
			static double getPolar( Face face, int32_t w, int32_t tu, int32_t tv, double &th, double &phi );

			CubeData() {}
			virtual ~CubeData() {}

			// �}�b�v�̃e�N�Z���T�C�Y���擾
			virtual uint32_t getTexelSize() const = 0;

			// �w���UV�ʒu�ɑ΂���l���擾
			virtual RGBA getValue( Face face, int32_t u, int32_t v ) const = 0;

			// �w���UV�ʒu�ɑ΂���XYZ���W���擾
			void getXYZ( Face face, int32_t tu, int32_t tv, double &x, double &y, double &z ) const;

			// �w���UV�ʒu�ɑ΂���ɍ��W���擾
			// �߂�l : �w��UV�܂ł̋���
			double getPolar( Face face, int32_t u, int32_t v, double &th, double &phi ) const;
		};

		// �p�����[�^
		class Parameter {
		public:
			bool operator ==( const Parameter &r ) const {
				return ( l_ == r.l_ && m_ == r.m_ );
			}
			bool operator <( const Parameter &r ) const {
				if ( l_ < r.l_ ) return true;
				else if ( l_ > r.l_ ) return false;
				return m_ < r.m_;
			}

		public:
			Parameter() {}
			Parameter( uint32_t l, int32_t m, double value ) : l_( l ), m_( m ), value_( value ) {}

			// �C���f�b�N�X���p�����[�^�ɕύX
			static void toLM( uint32_t idx, uint32_t &l, int32_t &m );

			// �p�����[�^���C���f�b�N�X�ɕύX
			static uint32_t toIdx( uint32_t l, int32_t m );

			// level�擾
			uint32_t l() const;

			// m�擾
			int32_t m() const;

			// �l�擾
			double value() const;

			// �L���ȃp�����[�^�H
			bool isValid() const;

		private:
			const static uint32_t invalidParam_g = 0xffffffff;
			uint32_t l_ = invalidParam_g;	// order
			int32_t m_ = 0;		// index in order( -l <= m <= l )
			double value_ = 0.0;	// ����l
		};

		// ������
		enum ResultState {
			RS_OK,				// ���萬��
			RS_NO_ESTIMATE,		// ����O
			RS_INVALID_DATA,	// ���̓f�[�^���s��
			RS_INVALID_PARAM,	// ���̓p�����[�^���s��
		};

		// �J���[
		enum ColorType {
			ColorType_R,
			ColorType_G,
			ColorType_B,
			ColorType_A
		};

		// ���茋��
		class Result {
		public:
			Result() {}
			Result( uint32_t maxLevel, const std::vector< std::vector< Parameter > > &params ) : maxLevel_( maxLevel ), paramsVec_( params ) {}
			~Result() {}

			// �����Ԃ��擾
			ResultState getState() const;

			// ���莞�̍ő�Level���擾
			uint32_t getMaxLevel() const;

			// �p�����[�^���X�g�擾
			const std::vector< Parameter > &getParamList( ColorType ctype ) const;

			// �w��C���f�b�N�X�̃p�����[�^���擾
			//  �߂�l : �����ȃp�����[�^���w�肳��Ă����ꍇ��Parameter::isValid��false
			Parameter getParam( ColorType ctype, uint32_t l, int32_t m ) const;

		private:
			uint32_t maxLevel_ = 0;
			std::vector< std::vector< Parameter > > paramsVec_;	// ����p�����[�^�i�F�ʁj
			ResultState state_ = ResultState::RS_NO_ESTIMATE;	// ������
		};

		// �G���[
		struct Error {
			bool error_ = false;	// true�ŃG���[����
			std::string reason_;
			Error() {}
			Error( const std::string &reason ) : error_( true ), reason_( reason ) {}
		};

		// ����x�[�X
		class Estimater {
		public:
			Estimater( uint32_t maxLevel ) : maxLevel_( maxLevel ) {}
			virtual ~Estimater() {}

			// ���莞��band order level�̍ő�l���擾
			uint32_t getMaxLevel() const;

		protected:
			uint32_t maxLevel_;	// ���ʒ��a�֐���band order�̍ő�l
		};

		// ����f�[�^����̃p�����[�^����
		class SphereEstimater : public Estimater {
		public:
			using Estimater::Estimater;
			virtual ~SphereEstimater() {}

			// ����
			//  �߂�l : �G���[�����������ꍇ�͗L�������񂪕Ԃ�
			Error estimate( const SphereData *sphere );
		};

		// �L���[�u�f�[�^����̃p�����[�^����
		class CubeEstimater : public Estimater {
		public:
			using Estimater::Estimater;
			virtual ~CubeEstimater() {}

			// ����
			Error estimate( const CubeData *cube, Result &res, const std::function< void( uint64_t count, uint64_t procCount ) > &proc );
		};

		// CubeMap�C���[�W����CubeData
		class CubeDataFromImage : public CubeData {
		public:
			using CubeData::CubeData;
			virtual ~CubeDataFromImage() {}

			// ������
			//  fileNames : 6�ʂ̃t�@�C����(�E�A���A�O�A��A��A���̏�)
			Error initialize( const std::vector< std::string > &fileNames );

			// �w���UV�ʒu�ɑ΂���l���擾
			virtual RGBA getValue( Face face, int32_t u, int32_t v ) const;

			// �}�b�v�̃e�N�Z���T�C�Y���擾
			virtual uint32_t getTexelSize() const override;

		private:
			ImageBlock images_[ 6 ];
		};

		// �p�����[�^�o��
		class OutputResult {
		public:
			OutputResult();
			virtual ~OutputResult();
			virtual Error output( const Result &result, const char* filePath );

		private:
			struct Header {
				uint32_t hederSize_ = 0;		// �w�b�_�[�T�C�Y
				uint32_t maxOrderLevel_ = 0;
				uint32_t componentListNum_ = 0;	// �e�F�̃��X�g��
				uint32_t containAlpha_ = 0;		// ��������ꍇ��1
				uint32_t reserved_ = 0;			// �\��̈�
			};
		};

		class OutputResultText : public OutputResult {
		public:
			OutputResultText();
			virtual ~OutputResultText();
			virtual Error output( const Result& result, const char* filePath );
		};

		// ���ʒ��a�֐��Q���쐬
		std::vector< std::function< double( double th, double phi )> > createSphericalHarmonicsFuncs( uint32_t level );

		// ����p�����[�^����L���[�u�}�b�v�쐬
		enum CubeMapType {
			Horizontal_Cross,	// ���N���X
			Vertical_Cross,		// �c�N���X
			Separable,			// 6�ʕ���
		};
		std::vector< ImageBlock > createCubeMapFromParameters( const Result &res, uint32_t width, CubeMapType mapType, const std::function< void( uint64_t count, uint64_t procCount ) > &proc );
	}
}

#endif
