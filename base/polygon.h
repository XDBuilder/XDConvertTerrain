#ifndef __POLYGON_H_INCLUDED__
#define __POLYGON_H_INCLUDED__

#include "xdmath.h"
//#include <float.h>
#include "vector3d.h"
#include "line2d.h"
#include "rect2d.h"
#include "gpc.h"

template <class T>
class polygon2d
{
public:

	vector2d<T>*	vertices;
	u32				vertexNumber; // member variables==========================================
	
	polygon2d() { vertices=0; vertexNumber=0; }; //fast than
	
	polygon2d( vector3d<T> *vec, u32 vnum ) : vertices(0),vertexNumber(0)
	{
		vertexNumber=vnum;
		if( vec && vnum>0 ){
			vertices = new vector2d<T>[vnum];
			for(u32 i=0;i<vnum;i++) vertices[i]=vector2d<T>(vec[i].x,vec[i].z);
		}
	};

	polygon2d( vector2d<T> *vec, u32 vnum )  : vertices(0),vertexNumber(0) 
	{
		vertexNumber = vnum;
		if( vec && vnum>0 ){
			vertices = new vector2d<T>[vnum];
			memcpy(vertices,vec,sizeof(vector2d<T>)*vnum);
		}
	};

//	polygon2d(const polygon2d<T>& other) : polygon2d( other.vertices, other.vertexNumber ) {};// �̰� �ȵ�(�����ڰ� �ΰ���)

	~polygon2d() { clear(); }

	void clear()
	{
		if(vertices){
			delete [] vertices;
			vertices=0;
		}
		vertexNumber=0;
	}

	void set( vector3d<T> *vec, u32 vnum )
	{
		clear();
		vertexNumber=vnum;
		if( vec && vnum>0 ){
			vertices = new vector2d<T>[vnum];
			for(u32 i=0;i<vnum;i++) vertices[i]=vector2d<T>(vec[i].x,vec[i].z);
		}

	}

	void set( vector2d<T> *vec, u32 vnum )
	{
		clear();
		vertexNumber=vnum;
		if( vec && vnum>0 ){
			vertices = new vector2d<T>[vnum];
			memcpy(vertices,vec,sizeof(vector2d<T>)*vnum);
		}
	}

	void setrect( vector2d<T> v1, vector2d<T> v2 )
	{
		clear();
		vertexNumber=4;
		vertices = new vector2d<T>[4];
		rect2d<T>  rect(v1.x,v1.y,v2.x,v2.y);
		rect.repair();
		vertices[0]=vector2d<T>(rect.ix,rect.iy);
		vertices[1]=vector2d<T>(rect.ix,rect.ay);
		vertices[2]=vector2d<T>(rect.ax,rect.ay);
		vertices[3]=vector2d<T>(rect.ax,rect.iy);
	}

	void setcircle( vector2d<T> vec, T r, int seg )
	{
		clear();
		vertexNumber=seg;
		vertices = new vector2d<T>[seg];
		for(int i=0; i<seg ;i++)
		{
			T theta=i*(PI*2/seg);
			vertices[i].x = vec.x + cos(theta)*r;
			vertices[i].y = vec.y + sin(theta)*r;
		}
	}

	void setlinebuffer( line2d<T> line, T r )
	{
		clear();
		vertexNumber=4;
		vertices = new vector2d<T>[vertexNumber];

		T sux = line.end.x - line.start.x;
		T suy = line.end.y - line.start.y;
		T sq = sqrt(sux*sux+suy*suy);

		vertices[0]=vector2d<T>(line.start.x-suy/sq*r,line.start.y+sux/sq*r);
		vertices[1]=vector2d<T>(line.start.x+suy/sq*r,line.start.y-sux/sq*r);
		vertices[2]=vector2d<T>(line.end.x+suy/sq*r,line.end.y-sux/sq*r);
		vertices[3]=vector2d<T>(line.end.x-suy/sq*r,line.end.y+sux/sq*r);
	}


	T getArea( )
	{
		u32 i,j;
		T area = 0;
		vector2d<T> opt,spt,ept;
		opt=vertices[0];
		for (i=0;i<vertexNumber;i++) {
			j = (i + 1) % vertexNumber;
			spt.x=opt.x-vertices[i].x;
			spt.y=opt.y-vertices[i].y;
			ept.x=opt.x-vertices[j].x;
			ept.y=opt.y-vertices[j].y;
			area += (spt.x*ept.y) - (ept.x*spt.y); 
		}
		area *=0.5f;

		return fabs(area);
	}

	T getLength( bool close )
	{
		u32 i,j,vnum=vertexNumber-1;
		T length = 0;
		if(close) vnum++;
		for ( i=0;i<vnum;i++) {
			j=(i+1)%vertexNumber;
			line2d<T> line(vertices[i],vertices[j]);
			length+=line.getLength();
		}
		return length;
	}

	int getBufferData( T r, int seg, bool close, vector2d<T>** vert, int& nPart, int** part,int** ptype )
	{
		nPart=0;
		if(vertexNumber==0) return 0;

		//������ ���������� ���۸������Ѵ�......
		polygon2d<T> pcircle,pline;
		gpc_polygon subject, result;

		int vnum=vertexNumber;
		if(close)//�������϶� �������� ������ �´�..
			gpc_read_polygon( vertices,1,&vnum,&result);
		else {//���� �����϶� ù��° ������ ���� ���´�..
			pcircle.setcircle( vertices[0], r, seg );
			gpc_read_polygon( pcircle.vertices,1,&seg,&result);
		}

		for(int i=0;i<vnum;i++)
		{
			//������ �ݰ�� ����׸�Ʈ ���� ���� ���Ѵ�..
			pcircle.setcircle( vertices[i], r, seg );
			gpc_read_polygon(pcircle.vertices,1,&seg,&subject);
			//������ ������� ������� ������ �����Ѵ�..
			gpc_polygon_clip(GPC_UNION, &subject, &result, &result);
			gpc_free_polygon(&subject);//������ ��� �޸� ����
		
			int pnum = 4;//���� �������� ���ε��� ���۸��� �������� ���Ѵ�.
			pline.setlinebuffer(line2d<T>(vertices[i],vertices[(i+1)%vnum]), r);
			gpc_read_polygon(pline.vertices,1,&pnum,&subject);
		
			if(close)//�������� ��..
				gpc_polygon_clip(GPC_UNION, &subject, &result, &result);
			else if( i<vnum-1 )//���� �����϶�
				gpc_polygon_clip(GPC_UNION, &subject, &result, &result);
		
			gpc_free_polygon(&subject);

		}
		vnum = gpc_point_count(&result);

		*vert = new vector2d<T>[vnum];
	
		nPart = gpc_polygon_count(&result);
		*part = new int[nPart];
		*ptype = new int[nPart];

		//������ ������� �ӽ÷� �����Ѵ�...
		gpc_write_polygon(*vert,*ptype,*part,&result);
		gpc_free_polygon(&result);//����� �޸𸮸� �����Ѵ�..

		return vnum;

	}

	vector2d<T> getMassCenter()//���� �߽�
	{
		vector2d<T> Ret(0.0, 0.0); // soo 20090410
		
		T dA = 0;	
		T dASum = 0;
		T dAGX = 0;
		T dAGY = 0;
		
		int next,n=vertexNumber;
		for(int i=0; i<n; i++){
			next=(i+1)%n;
			dA = (vertices[i].x * vertices[next].y) - (vertices[next].x * vertices[i].y);
			dASum += dA / 2.0;
			dAGX += (vertices[i].x + vertices[next].x) * dA;
			dAGY += (vertices[i].y + vertices[next].y) * dA;
		}
		
		Ret.x = dAGX / (dASum * 6);
		Ret.y = dAGY / (dASum * 6);  
		
		return Ret;
	}
	//�� �迭 ������ �ð� �������� �ݽð� �������� �˻�
	int clockWise()//3D �� �ִ°͵� ������ �� �ҵ�
	{
		if(vertexNumber<3) return -1;//error
		
		T Term;
		T aSum=0.0f;
		T xSum=0.0f;
		T ySum=0.0f;
		int i,j,n=vertexNumber;
		for(i=0;i<n;i++){
			j = (i + 1) % n;
			Term = vertices[i].x * vertices[j].y  -  vertices[j].x * vertices[i].y;
			aSum = aSum + Term;
			xSum = xSum + (vertices[j].x + vertices[i].x) * Term;
			ySum = ySum + (vertices[j].y + vertices[i].y) * Term;
		}
		if(aSum<0) return 1;//�ð����(cw)
		return 0;//�ݽð����(ccw)
	}
	//�Ʒ��� �����￡ ���� ���� üũ�� 4���� �߽��� �� üũ�ϴ� ����. ������ üũ�� ���� �����ؾ���.
	//5 : rect �� ��� �����￡ ����
	//1~4 : rect �Ϻΰ� �����￡ ���� 
	//0 : rect �� ��� �����￡ ���� �ȵ�.
	short isRectInside(const rect2d<T>& rect) 
	{
		short ret=0;
		vector2d<T> *pts = new vector2d<T>[5] ;
		pts[0].x = rect.ix; pts[0].y = rect.iy;
		pts[1].x = rect.ix; pts[1].y = rect.ay;
		pts[2].x = rect.ax; pts[2].y = rect.ay;
		pts[3].x = rect.ax; pts[3].y = rect.iy;
		pts[4].x = (rect.ax+rect.ix)/2.0f; pts[4].y = (rect.ay+rect.iy)/2.0f;
		bool check =false;
		for(int i=0;i<5;i++) {
			if(isPointInside(pts[i].x,pts[i].y)==true) {
//				delete [] pts;
//				pts = 0;
				ret++;
			}
		}
		delete [] pts;
		pts = 0;
		return ret;
	}
	// ���� �����￡ ���ԵǴ��� �˻�
	bool isPointInside( T qp_x,T qp_y )
	{
		int i, j;
		bool c = false;
		
		for (i = 0, j = vertexNumber-1; i < vertexNumber; j = i++) 
		{
			if ( ((vertices[i].y>qp_y) != (vertices[j].y>qp_y)) &&
				 (qp_x < (vertices[j].x-vertices[i].x) * (qp_y-vertices[i].y) / (vertices[j].y-vertices[i].y) + vertices[i].x) )
				c = !c;
		}

		if(c==false) // ���� ���� �ִ��� �˻� soo 20100104
		{
			line2d<T> line;
			vector2d<T> qp( qp_x, qp_y );
			for (i = 0 ; i < vertexNumber; i++) 
			{
				line.start = vertices[i];
				if(i==vertexNumber-1) line.end = vertices[0]; else line.end = vertices[i+1];
				if(line.isPointOnLine(qp)) return true;// ���� ���� ������ 
			}
		}
		return c;
/*

		T	x;
		int i,i1;
		int Rcross=0;       //������ ������ ���� ������ ������
		int Lcross=0;       //���� ������ ���� ������ ������
		bool Rstrad,Lstrad;
		
		int n=v;
		vector2d<T>* vec = new vector2d<T>[n];

		vector2d<T> qp( qp_x, qp_y );
		//QueryPoint�� ������ �̵�
		for(i=0;i<n;i++) vec[i] = vertices[i] - qp;

		//�����￡ �ִ� ������ ����(�𼭸�)�� ������ �����ϴ��� �˻�
		for(i=0; i<n;i++)
		{
			//������ ������ �ϳ����� Ȯ�� 
			if ( (vec[i].x == 0) & (vec[i].y == 0) ) {
				delete [] vec;
				return true; //�������� ������ �ϳ�  
			}
			
			i1 = (( i + n - 1) % n);
			
			Rstrad=(( vec[i].y > 0 ) != ( vec[i1].y > 0 ));
			Lstrad=(( vec[i].y < 0 ) != ( vec[i1].y < 0 )); 
			
			if ( Rstrad | Lstrad ){
				// X��� �����ϴ� �� ���� 
				x = (vec[i].x * vec[i1].y - vec[i1].x * vec[i].y) /(vec[i1].y - vec[i].y);
				if ( ( Rstrad ) & ( x > 0 ) )  Rcross++; //�������� > 0 �� ������ 
				if ( ( Lstrad ) & ( x < 0 ) )  Lcross++; //�������� < 0 �� ���� 
			}
		}
		
		delete [] vec;
		
		//   msgbox("������ : %d,���� : %d",Rcross,Lcross);
		
		if ( ( Rcross % 2 ) != ( Lcross % 2 ) ) return false;  //"Ȧ Ȧ" �Ǵ� "¦ ¦" �� �ƴϸ� ���� 
		
		if ( ( Rcross % 2 ) == 1 ) 	return true; //������ ���ο� ���� �ִ�. (Ȧ Ȧ) �ΰ�� 
		
		return false; //������ �ܺο� ���� �ִ�. (¦ ¦)�ΰ�� 
*/
	}

	T isLeft( vector2d<T> P0, vector2d<T> P1, vector2d<T> P2 )
	{
		return (P1.x - P0.x)*(P2.y - P0.y) - (P2.x - P0.x)*(P1.y - P0.y);
	}
	// simpleHull_2D():
	// Input:  V[] = polyline array of 2D vertex points 
	// n   = the number of points in V[]
	// Output: H[] = output convex hull array of vertices (max is n)
	// Return: h   = the number of points in H[]
	vector2d<T>* convexHull( int& outnum )
	{
		// initialize a deque D[] from bottom to top so that the
		// 1st three vertices of V[] are a counterclockwise triangle
		if(vertices==NULL || vertexNumber==0) return 0;

		int n=vertexNumber;
		vector2d<T>* D = new vector2d<T>[2*n+1];
	
		int bot = n-2, top = bot+3;   // initial bottom and top deque indices
		D[bot] = D[top] = vertices[2];       // 3rd vertex is at both bot and top
		if (isLeft(vertices[0], vertices[1], vertices[2]) > 0) {
			D[bot+1] = vertices[0];
			D[bot+2] = vertices[1];          // ccw vertices are: 2,0,1,2
		}
		else {
			D[bot+1] = vertices[1];
			D[bot+2] = vertices[0];          // ccw vertices are: 2,1,0,2
		}
		int h;        // hull vertex counter
		// compute the hull on the deque D[]
		for (int i=3; i < n; i++) {   // process the rest of vertices
			// test if next vertex is inside the deque hull
			if ((isLeft(D[bot], D[bot+1], vertices[i]) > 0) &&	(isLeft(D[top-1], D[top], vertices[i]) > 0) )
                continue;         // skip an interior vertex
			
			// incrementally add an exterior vertex to the deque hull
			// get the rightmost tangent at the deque bot
			while (isLeft(D[bot], D[bot+1], vertices[i]) <= 0)	++bot;                // remove bot of deque
			D[--bot] = vertices[i];          // insert V[i] at bot of deque
			
			// get the leftmost tangent at the deque top
			while (isLeft(D[top-1], D[top], vertices[i]) <= 0)	--top;                // pop top of deque
			D[++top] = vertices[i];          // push V[i] onto top of deque
		}
		
		// transcribe deque D[] to the output hull array H[]
		vector2d<T>* H = new vector2d<T>[(top-bot)+1];

		for (h=0; h <= (top-bot); h++)	H[h] = D[bot + h];
		
		if(D) delete [] D;

		outnum = h-1;

		return H;
	} 


};
	
typedef polygon2d<f32> polygon2df;// Typedef for f32 3d vector.
typedef polygon2d<f64> polygon2dd;// Typedef for f64 3d vector.
typedef polygon2d<s32> polygon2di;// Typedef for integer 3d vector.

#endif

