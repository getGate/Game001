﻿#include "GameLib/GameLib.h"
#include "GameLib/Scene/PrimitiveRenderer.h"
#include "GameLib/Graphics/Manager.h"
#include "GameLib/Graphics/Vertex.h"
#include "GameLib/Graphics/VertexBuffer.h"
#include "GameLib/Graphics/Texture.h"
#include "GameLib/Math/Matrix34.h"
#include "GameLib/Math/Matrix44.h"
#include "GameLib/Math/Vector4.h"
#include "GameLib/Math/Vector3.h"
#include "GameLib/Math/Vector2.h"
#include "GameLib/Base/Queue.h"
#include "GameLib/Base/Impl/ReferenceType.h"

namespace GameLib{
using namespace Math;
using namespace Graphics;

namespace Scene{

class PrimitiveRenderer::Impl : public ReferenceType{
public:
	enum Type{
		TYPE_TRIANGLE_TRANSFORMED,
		TYPE_TRIANGLE,
		TYPE_LINE,
		TYPE_POINT,
		TYPE_RECTANGLE,
		TYPE_UNKNOWN,
	};
	struct Batch{
		Batch() :
		mDepthWrite( true ),
		mDepthTest( false ),
		mBlendMode( BLEND_OPAQUE ),
		mCullMode( CULL_NONE ),
		mType( TYPE_UNKNOWN ),
		mStart( 0 ),
		mPrimitiveNumber( 0 ){
			mTransform.setIdentity();
		}

		Matrix44 mTransform;
		Texture mTexture;
		bool mDepthWrite;
		bool mDepthTest;
		BlendMode mBlendMode;
		CullMode mCullMode;
		Type mType;
		int mStart;
		int mPrimitiveNumber;
	};

	Impl( 
	int vertexCapacity,
	int batchCapacity ) :
	mVertexCapacity( vertexCapacity ),
	mBatchCapacity( batchCapacity + 1 ),//dummy+1
	mVertexPosition( 0 ),
	mBatchPosition( 0 ),
	mBatches( 0 ),
	mLockedVertexBuffer( 0 ){
		ASSERT( mVertexCapacity > 0 && "can't specify ZERO capacity" );
		ASSERT( mBatchCapacity > 0 && "can't specify ZERO capacity" );
		mVertexBuffer = VertexBuffer::create( mVertexCapacity );
		mLockedVertexBuffer = mVertexBuffer.lock(); //第一次锁

		mBatches = NEW Batch[ mBatchCapacity ];
		mBatches[ 0 ].mPrimitiveNumber = 0x7fffffff; //0号码是假批量
		mPreviousFrameId = Manager().frameId() - 1; //通过的值
	}
	~Impl(){
		SAFE_DELETE_ARRAY( mBatches );
		if ( mLockedVertexBuffer ){
			mVertexBuffer.unlock( &mLockedVertexBuffer );
		}
	}
	Batch* addBatch(){
		ASSERT( mBatchPosition + 1 < mBatchCapacity );
		mBatches[ mBatchPosition + 1 ] = mBatches[ mBatchPosition ]; //复制
		++mBatchPosition;
		Batch* b = &mBatches[ mBatchPosition  ];
		b->mPrimitiveNumber = 0;
		b->mStart = mVertexPosition;
		return b;
	}
	void setTexture( Texture& t ){
		Batch* b = &mBatches[ mBatchPosition ];
		if ( b->mTexture != t ){
			if ( b->mPrimitiveNumber > 0 ){
				b = addBatch();
			}
			b->mTexture = t;
		}
	}
	void enableDepthTest( bool f ){
		Batch* b = &mBatches[ mBatchPosition ];
		if ( b->mDepthTest != f ){
			if ( b->mPrimitiveNumber > 0 ){
				b = addBatch();
			}
			b->mDepthTest = f;
		}
	}
	void enableDepthWrite( bool f ){
		Batch* b = &mBatches[ mBatchPosition ];
		if ( b->mDepthWrite != f ){
			if ( b->mPrimitiveNumber > 0 ){
				b = addBatch();
			}
			b->mDepthWrite = f;
		}
	}
	void setBlendMode( Graphics::BlendMode bm ){
		Batch* b = &mBatches[ mBatchPosition ];
		if ( b->mBlendMode != bm ){
			if ( b->mPrimitiveNumber > 0 ){
				b = addBatch();
			}
			b->mBlendMode = bm;
		}
	}
	void setCullMode( Graphics::CullMode cm ){
		Batch* b = &mBatches[ mBatchPosition ];
		if ( b->mCullMode != cm ){
			if ( b->mPrimitiveNumber > 0 ){
				b = addBatch();
			}
			b->mCullMode = cm;
		}
	}
	void setTransform( const Matrix44& m ){
		Batch* b = &mBatches[ mBatchPosition ];
		if ( b->mTransform != m ){
			if ( b->mPrimitiveNumber > 0 ){
				b = addBatch();
			}
			b->mTransform = m;
		}
	}
	void addTransformedTriangle(
	const Vector4& p0,
	const Vector4& p1,
	const Vector4& p2,
	const Vector2& t0,
	const Vector2& t1,
	const Vector2& t2,
	unsigned c0,
	unsigned c1,
	unsigned c2 ){
		ASSERT( ( mVertexPosition + 3 ) <= mVertexCapacity );
		ASSERT( mLockedVertexBuffer );

		Batch* b = &mBatches[ mBatchPosition ];
		if ( b->mPrimitiveNumber > 0 ){
			if ( b->mType != TYPE_TRIANGLE_TRANSFORMED ){
				b = addBatch();
			}
		}
		b->mType = TYPE_TRIANGLE_TRANSFORMED;
		++b->mPrimitiveNumber;

		Vertex* v = mLockedVertexBuffer + mVertexPosition;

		v[ 0 ].mPosition = p0;
		v[ 0 ].mNormal = 0.f;
		v[ 0 ].mColor = c0;
		v[ 0 ].mUv = t0;

		v[ 1 ].mPosition = p1;
		v[ 1 ].mNormal = 0.f;
		v[ 1 ].mColor = c1;
		v[ 1 ].mUv = t1;

		v[ 2 ].mPosition = p2;
		v[ 2 ].mNormal = 0.f;
		v[ 2 ].mColor = c2;
		v[ 2 ].mUv = t2;
		
		mVertexPosition += 3;
	}
	void addTriangle(
	const Vector3& p0,
	const Vector3& p1,
	const Vector3& p2,
	const Vector2& t0,
	const Vector2& t1,
	const Vector2& t2,
	unsigned c0,
	unsigned c1,
	unsigned c2 ){
		ASSERT( ( mVertexPosition + 3 ) <= mVertexCapacity );
		ASSERT( mLockedVertexBuffer );

		Batch* b = &mBatches[ mBatchPosition ];
		if ( b->mPrimitiveNumber > 0 ){
			if ( b->mType != TYPE_TRIANGLE ){
				b = addBatch();
			}
		}
		b->mType = TYPE_TRIANGLE;
		++b->mPrimitiveNumber;

		Vertex* v = mLockedVertexBuffer + mVertexPosition;

		v[ 0 ].mPosition = p0;
		v[ 0 ].mNormal = 0.f;
		v[ 0 ].mColor = c0;
		v[ 0 ].mUv = t0;

		v[ 1 ].mPosition = p1;
		v[ 1 ].mNormal = 0.f;
		v[ 1 ].mColor = c1;
		v[ 1 ].mUv = t1;

		v[ 2 ].mPosition = p2;
		v[ 2 ].mNormal = 0.f;
		v[ 2 ].mColor = c2;
		v[ 2 ].mUv = t2;
		
		mVertexPosition += 3;
	}
	void addLine(
	const Vector3& p0,
	const Vector3& p1,
	const Vector2& t0,
	const Vector2& t1,
	unsigned c0,
	unsigned c1 ){
		ASSERT( ( mVertexPosition + 2 ) <= mVertexCapacity );
		ASSERT( mLockedVertexBuffer );

		Batch* b = &mBatches[ mBatchPosition ];
		if ( b->mPrimitiveNumber > 0 ){
			if ( b->mType != TYPE_LINE ){
				b = addBatch();
			}
		}
		b->mType = TYPE_LINE;
		++b->mPrimitiveNumber;

		Vertex* v = mLockedVertexBuffer + mVertexPosition;

		v[ 0 ].mPosition = p0;
		v[ 0 ].mNormal = 0.f;
		v[ 0 ].mColor = c0;
		v[ 0 ].mUv = t0;

		v[ 1 ].mPosition = p1;
		v[ 1 ].mNormal = 0.f;
		v[ 1 ].mColor = c1;
		v[ 1 ].mUv = t1;

		mVertexPosition += 2;
	}
	void addPoint(
	const Vector3& p,
	const Vector2& t,
	unsigned c ){
		ASSERT( ( mVertexPosition + 1 ) <= mVertexCapacity );
		ASSERT( mLockedVertexBuffer );

		Batch* b = &mBatches[ mBatchPosition ];
		if ( b->mPrimitiveNumber > 0 ){
			if ( b->mType != TYPE_POINT ){
				b = addBatch();
			}
		}
		b->mType = TYPE_POINT;
		++b->mPrimitiveNumber;

		Vertex* v = mLockedVertexBuffer + mVertexPosition;

		v[ 0 ].mPosition = p;
		v[ 0 ].mNormal = 0.f;
		v[ 0 ].mColor = c;
		v[ 0 ].mUv = t;

		++mVertexPosition;
	}
	void addRectangle(
	const Vector2& p0,
	const Vector2& p1,
	const Vector2& t0,
	const Vector2& t1,
	unsigned c,
	float depth ){
		ASSERT( ( mVertexPosition + 6 ) <= mVertexCapacity );
		ASSERT( mLockedVertexBuffer );

		Batch* b = &mBatches[ mBatchPosition ];
		if ( b->mPrimitiveNumber > 0 ){
			if ( b->mType != TYPE_RECTANGLE ){
				b = addBatch();
			}
		}
		b->mType = TYPE_RECTANGLE;
		b->mPrimitiveNumber += 2; //请注意，有两个三角形

		Vertex* v = mLockedVertexBuffer + mVertexPosition;

		for ( int i = 0; i < 6; ++i ){
			v[ i ].mColor = c;
			v[ i ].mNormal = 0.f;
		}

		v[ 0 ].mPosition.set( p0.x, p0.y, depth );
		v[ 0 ].mUv.set( t0.x, t0.y );
		v[ 1 ].mPosition.set( p0.x, p1.y, depth );
		v[ 1 ].mUv.set( t0.x, t1.y );
		v[ 2 ].mPosition.set( p1.x, p0.y, depth );
		v[ 2 ].mUv.set( t1.x, t0.y );

		v[ 3 ].mPosition.set( p1.x, p0.y, depth );
		v[ 3 ].mUv.set( t1.x, t0.y );
		v[ 4 ].mPosition.set( p0.x, p1.y, depth );
		v[ 4 ].mUv.set( t0.x, t1.y );
		v[ 5 ].mPosition.set( p1.x, p1.y, depth );
		v[ 5 ].mUv.set( t1.x, t1.y );

		mVertexPosition += 6;
	}
	void draw(){
		//帧ID检查
		unsigned fid = Manager().frameId();
		ASSERT( mPreviousFrameId != fid && "PrimitiveRenderer::draw() : you can't draw() twice in a frame!" );
		mPreviousFrameId = fid;
		//顶点缓冲区解锁
		mVertexBuffer.unlock( &mLockedVertexBuffer );
		//别名
		Manager m;
		//设置顶点缓冲区
		m.setVertexBuffer( mVertexBuffer );
		//设置初始值并转动。
		Matrix44 transform;
		transform.setIdentity();
		Matrix34 wm;
		wm.setIdentity();
		Texture texture;
		bool depthWrite = true;
		bool depthTest = false;
		BlendMode blendMode = BLEND_OPAQUE;
		CullMode cullMode = CULL_NONE;
		m.setProjectionViewMatrix( transform );
		m.setWorldMatrix( wm );
		m.setTexture( texture );
		m.enableDepthWrite( depthWrite );
		m.enableDepthTest( depthTest );
		m.setBlendMode( blendMode );
		m.setCullMode( cullMode );
		m.setDiffuseColor( Vector3( 1.f, 1.f, 1.f ) );
		m.setLightingMode( LIGHTING_NONE );
		//屏幕坐标->clip空间转换矩阵
		Matrix44 screenToClipTransform;
		int width;
		int height;
		m.getViewport( 0, 0, &width, &height );
		screenToClipTransform.setOrthogonalTransform(
			0.f, static_cast< float >( width ), 
			static_cast< float >( height ), 0.f, //
			0.f, 1.f );
		//单位矩阵
		Matrix44 identityTransform;
		identityTransform.setIdentity();
		//放入特殊矩阵后
		bool specialTransform = false;

		//一个接一个地处理批次。由于dummy被忽略，因此从第一个开始
		for ( int i = 1; i <= mBatchPosition; ++i ){
			Batch& b = mBatches[ i ];
			//注入非矩阵状态
			if ( b.mDepthWrite != depthWrite ){
				depthWrite = b.mDepthWrite;
				m.enableDepthWrite( depthWrite );
			}
			if ( b.mDepthTest != depthTest ){
				depthTest = b.mDepthTest;
				m.enableDepthTest( depthTest );
			}
			if ( b.mBlendMode != blendMode ){
				blendMode = b.mBlendMode;
				m.setBlendMode( blendMode );
			}
			if ( b.mCullMode != cullMode ){
				cullMode = b.mCullMode;
				m.setCullMode( cullMode );
			}
			if ( b.mTexture != texture ){
				texture = b.mTexture;
				m.setTexture( texture );
			}
			//只有矩阵有点特殊
			//首先，如果有所不同，则更新为相同
			bool transformChanged = false;
			if ( b.mTransform != transform ){
				transform = b.mTransform;
				transformChanged = true;
			}
			//是否设置取决于原始类型
			if ( b.mType == TYPE_RECTANGLE ){ //设置clip空间矩阵
				m.setProjectionViewMatrix( screenToClipTransform );
				specialTransform = true;
			}else if ( b.mType == TYPE_TRIANGLE_TRANSFORMED ){
				m.setProjectionViewMatrix( identityTransform );
				specialTransform = true;
			}else if ( specialTransform || transformChanged ){ //
				m.setProjectionViewMatrix( transform );
				specialTransform = false;
			}
			//绘制
			if ( b.mType != TYPE_UNKNOWN ){
				PrimitiveType prim = PRIMITIVE_TRIANGLE;
				switch ( b.mType ){
					case TYPE_TRIANGLE_TRANSFORMED:
					case TYPE_TRIANGLE:
					case TYPE_RECTANGLE:
						prim = PRIMITIVE_TRIANGLE;
						break;
					case TYPE_LINE:
						prim = PRIMITIVE_LINE;
						break;
					case TYPE_POINT:
						prim = PRIMITIVE_POINT;
						break;
					default: ASSERT( 0 ); break;
				}
				m.draw( b.mStart, b.mPrimitiveNumber, prim );
			}
			//丢弃批处理中的所有纹理手柄
			b.mTexture.release();
		}
		//顶点缓冲区锁定下一个
		m.setVertexBuffer( 0 ); //删除
		mLockedVertexBuffer = mVertexBuffer.lock();
		//之后清理
		mVertexPosition = 0;
		mBatchPosition = 0;
		//纹理清理（除非完成，否则不能销毁）
		m.setTexture( 0 );
	}
private:
	void operator=( const Impl& ); //禁止

	const int mVertexCapacity;
	const int mBatchCapacity;
	int mVertexPosition;
	int mBatchPosition;
	Batch* mBatches;
	Graphics::Vertex* mLockedVertexBuffer;	
	unsigned mPreviousFrameId;

	Graphics::VertexBuffer mVertexBuffer;
};

PrimitiveRenderer PrimitiveRenderer::create( int tc, int cc ){
	PrimitiveRenderer r;
	r.mImpl = NEW Impl( tc, cc );
	return r;
}

void PrimitiveRenderer::setTexture( Graphics::Texture t ){
	mImpl->setTexture( t );
}

void PrimitiveRenderer::enableDepthTest( bool f ){
	mImpl->enableDepthTest( f );
}

void PrimitiveRenderer::enableDepthWrite( bool f ){
	mImpl->enableDepthWrite( f );
}

void PrimitiveRenderer::setBlendMode( Graphics::BlendMode b ){
	mImpl->setBlendMode( b );
}

void PrimitiveRenderer::setCullMode( Graphics::CullMode c ){
	mImpl->setCullMode( c );
}

void PrimitiveRenderer::setTransform( const Matrix44& m ){
	mImpl->setTransform( m );
}

void PrimitiveRenderer::addTransformedTriangle(
const Vector4& p0,
const Vector4& p1,
const Vector4& p2,
const Vector2& t0,
const Vector2& t1,
const Vector2& t2,
unsigned c0,
unsigned c1,
unsigned c2 ){
	mImpl->addTransformedTriangle( p0, p1, p2, t0, t1, t2, c0, c1, c2 ); 
}

void PrimitiveRenderer::addTransformedTriangle(
const Vector4& p0,
const Vector4& p1,
const Vector4& p2,
unsigned c0,
unsigned c1,
unsigned c2 ){
	Vector2 t( 0.f, 0.f ); //空的UV
	mImpl->addTransformedTriangle( p0, p1, p2, t, t, t, c0, c1, c2 ); 
}

void PrimitiveRenderer::addTriangle(
const Vector3& p0,
const Vector3& p1,
const Vector3& p2,
const Vector2& t0,
const Vector2& t1,
const Vector2& t2,
unsigned c0,
unsigned c1,
unsigned c2 ){
	mImpl->addTriangle( p0, p1, p2, t0, t1, t2, c0, c1, c2 ); 
}

void PrimitiveRenderer::addTriangle(
const Vector3& p0,
const Vector3& p1,
const Vector3& p2,
unsigned c0,
unsigned c1,
unsigned c2 ){
	Vector2 t( 0.f, 0.f ); //空的UV
	mImpl->addTriangle( p0, p1, p2, t, t, t, c0, c1, c2 ); 
}

void PrimitiveRenderer::addLine(
const Vector3& p0,
const Vector3& p1,
const Vector2& t0,
const Vector2& t1,
unsigned c0,
unsigned c1 ){
	mImpl->addLine( p0, p1, t0, t1, c0, c1 ); 
}

void PrimitiveRenderer::addLine(
const Vector3& p0,
const Vector3& p1,
unsigned c0,
unsigned c1 ){
	Vector2 t( 0.f, 0.f ); //空的UV
	mImpl->addLine( p0, p1, t, t, c0, c1 ); 
}

void PrimitiveRenderer::addPoint(
const Vector3& p,
const Vector2& t,
unsigned c ){
	mImpl->addPoint( p, t, c ); 
}

void PrimitiveRenderer::addPoint(
const Vector3& p,
unsigned c ){
	Vector2 t( 0.f, 0.f ); //空的UV
	mImpl->addPoint( p, t, c ); 
}

void PrimitiveRenderer::addRectangle(
const Vector2& p0,
const Vector2& p1,
const Vector2& t0,
const Vector2& t1,
unsigned c,
float depth ){
	mImpl->addRectangle( p0, p1, t0, t1, c, depth ); 
}

void PrimitiveRenderer::addRectangle(
const Vector2& p0,
const Vector2& p1,
unsigned c,
float depth ){
	Vector2 t( 0.f, 0.f );
	mImpl->addRectangle( p0, p1, t, t, c, depth ); 
}

void PrimitiveRenderer::draw(){
	mImpl->draw();
}

#define TYPE PrimitiveRenderer
#include "GameLib/Base/Impl/ReferenceTypeTemplate.h"

} //namespace PseudoXml
} //namespace GameLib
