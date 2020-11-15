#include<gl/GLUT.H>
#include<iostream>
#include<vector>
#include<math.h>


using namespace std;

//int win;
int width = 600;
int height = 400;
int xcur, ycur = 0; //cursor's (x,y)
int cur_state = 0; //current state:1--dot, 2--line, 3--polygon, 4--circle, 5--clear, 6--redraw
//bool line_p1 = false; //is this click the first point of a line?
struct Point {
	int x;
	int y;
};
struct Line {
	Point p1;
	Point p2;
};
struct Circle {
	Point center;//圓心
	float radius;//半徑
};

vector<Point> points;
vector<Point> throw_points;
//---------------------------------------------------------------
vector<Line> lines;
vector<Line> throw_lines;
Point linepoint1;//first point of line
Point linepoint2;
int linefirstp = 0; //0:start a new line; 1:the first point
//---------------------------------------------------------------
vector<vector<Line>> polygons;
vector<vector<Line>> throw_polygons;
vector<Line> curpolyline;//current polygon's lines 
Point polypoint1;//first point of polygon
Point polylinefirstpoint;
bool polystart = false;
//---------------------------------------------------------------
vector<Circle> circles;
vector<Circle> throw_circles;
bool circlestart = false;
Point circlefirstpoint;
//int c_state = 0;

//https://www.cnblogs.com/wsine/p/4639050.html
void drawcircle(float r, Point center)
{
	float x = 0;
	float y = r;
	float d = 1 - r;
	float incE = 3.0;
	float incSE = (-2)*r + 5;
	while (x < y) {
		if (d < 0) {//choose E
			d += incE;
			incE += 2;
			incSE += 2;
			x++;
		}
		else {
			d += incSE;
			incE += 2;
			incSE += 4;
			x++;
			y--;
		}
		glVertex2f(x + center.x, y + center.y);
		glVertex2f(y + center.x, x + center.y);
		glVertex2f(-y + center.x, x + center.y);
		glVertex2f(-x + center.x, y + center.y);
		glVertex2f(x + center.x, -y + center.y);
		glVertex2f(y + center.x, -x + center.y);
		glVertex2f(-x + center.x, -y + center.y);
		glVertex2f(-y + center.x, -x + center.y);
	}//while ends
}

void drawline(float x1, float y1, float x2, float y2)
{
	cout << "---------draw line from " << x1 << "," << y1 << " to " << x2 << "," << y2 << "----------\n";
	//glVertex2f(x1, y1);
	//glVertex2f(x2, y2);
	//float x = x1;
	//float y = y1;
	float a = y2 - y1;
	float b = x2 - x1;
	float inc = 0.0;
	float d = 0;
	if (x1 == x2) {//縱線
		if (a >= 0) {//y2 >= y1 線由下到上
			inc = y1;
			while (inc <= y2) {
				glVertex2f(x1, inc);
				inc++;
			}
		}
		else {		//y1 > y2 線由上到下
			inc = y2;
			while (inc <= y1) {
				glVertex2f(x1, inc);
				inc++;
			}
		}
	}
	else if (y1 == y2) {//橫線
		if (b >= 0) {//x1 <= x2
			inc = x1;
			while (inc <= x2) {
				glVertex2f(inc, y1);
				inc++;
			}
		}
		else {       //x2 < x1
			inc = x2;
			while (inc <= x1) {
				glVertex2f(inc, y1);
				inc++;
			}
		}
	}
	else {
		float m = a / b; //斜率
		glVertex2f(x1, y1);
		if (0 <= m && m <= 1) {//cosine--0~45度
			if ((x2 < x1) && (y2 < y1)) {//都從同一方向填充
				float tx = x1;
				x1 = x2;
				x2 = tx;
				float ty = y1;
				y1 = y2;
				y2 = ty;
			}
			a = y2 - y1;
			b = -(x2 - x1);
			d = a + (b / 2);
			glVertex2f(x1, y1);
			while (x1 < x2) {
				if (d <= 0) {//choose the point on the EAST side
					x1++;
					d += a;
				}
				else {//choose the point on the NORTH-EAST side
					x1++;
					y1++;
					d += (a + b);
				}
				glVertex2f(x1, y1);
			}
		}
		else if (m > 1) {//cosine--45~90度
			if ((x2 < x1) && (y2 < y1)) {//都從同一方向填充
				float tx = x1;
				x1 = x2;
				x2 = tx;
				float ty = y1;
				y1 = y2;
				y2 = ty;
			}
			a = y2 - y1;
			b = -(x2 - x1);//x1-x2
			d = (-b) - (a / 2); //y2-y1的差距比0~45度的y2-y1還要大，所以要除以2(平衡)
			glVertex2f(x1, y1);
			while (y1 < y2) {
				if (d <= 0) {//choose the point on the NORTH side--cosine:60~90
					y1++;
					d -= b; //d = d - (x2 - x1) because choosing the point on the north side, the next mid-point will have to 減掉 x-coodinate 的差距，保持在60度以上
				}
				else {		 //choose the point on the NORTH-EAST side--cosine:45~60
					x1++;
					y1++;
					d -= (a + b);
				}
				glVertex2f(x1, y1);
			}
		}
		else if (m < -1) {//cosine--90~135度
			if ((x2 > x1) && (y2 < y1)) {//都從同一方向填充
				float tx = x1;
				x1 = x2;
				x2 = tx;
				float ty = y1;
				y1 = y2;
				y2 = ty;
			}
			a = y2 - y1;
			b = -(x2 - x1);//x1-x2
			d = b - a / 2; //y2-y1的差距比0~45度的y2-y1還要大，所以要除以2(平衡)
			glVertex2f(x1, y1);
			while (y1 < y2) {
				if (d <= 0) {//choose the point on the NORTH side--cosine:90~120
					y1++;
					d += b; //choosing the point on the north side, the next mid-point will have to 減掉 x-coodinate 的差距，保持在60度以上，
				}			//直接加x1-x2是因為在第二象限這樣加就直接是複數了
				else {//choose the point on the NORTH side--cosine:120~135
					y1++;
					x1--;
					d += (b - a);
				}
				glVertex2f(x1, y1);
			}
		}
		else if (-1 <= m && m <= 0) {//cosine--135~180度
			if ((x2 > x1) && (y2 < y1)) {//都從同一方向填充
				float tx = x1;
				x1 = x2;
				x2 = tx;
				float ty = y1;
				y1 = y2;
				y2 = ty;
			}
			a = y2 - y1;
			b = -(x2 - x1);//x1-x2
			d = b / 2 - a;  //x2-x1的差距比0~45度的x2-x1還要大，所以要除以2(平衡)；
			glVertex2f(x1, y1);
			while (x2 < x1) {
				if (d >= 0) {//choose the point on the WEST side--cosine:150~180
					x1--;
					d -= a;
				}
				else {       //choose the point on the NORTH-WEST side--cosine:90~150
					y1++;
					x1--;
					d += (b - a);
				}
				glVertex2f(x1, y1);
			}
		}
	}

}

void MyKeyboard(unsigned char key, int x, int y)
{
	//Q q esc
	if (key == 81 || key == 113 || key == 27) {
		cout << "quit\n";
		exit(0);
		//glutDestroyWindow(win);
	}
	//d D
	if (key == 100 || key == 68) {
		cout << "dot\n";
		cur_state = 1;
	}
	//L l
	else if (key == 76 || key == 108) {//https://birunthag.blogspot.com/2014/05/openglc-draw-line-with-mouse-clicking.html
		cout << "line\n";
		linefirstp = 0;//set true because the next point are gonna be the first point of line
		cur_state = 2;
	}
	//p P
	else if (key == 112 || key == 80) {
		cout << "polygon\n";
		cur_state = 3;
		//polystart = true;
	}
	//o O
	else if (key == 111 || key == 79) {
		cout << "circle\n";
		cur_state = 4;
	}
	//clear c C
	else if (key == 99 || key == 67) {
		cout << "clear\n";
		//cur_state = 5; //去掉這一行，這樣按下C時只會執行清除的動作，功能則會維持在原本的畫畫功能，不用再按一次功能選項鍵
		if (points.size() != 0) {
			for (vector<Point>::iterator i = points.begin(); i != points.end(); ++i) {
				throw_points.push_back(*i);
			}
			points.clear();
		}
		if (lines.size() != 0) {
			for (vector<Line>::iterator i = lines.begin(); i != lines.end(); ++i) {
				throw_lines.push_back(*i);
			}
			lines.clear();
		}
		if (polygons.size() != 0) {
			for (vector<vector<Line>>::iterator i = polygons.begin(); i != polygons.end(); ++i) {
				throw_polygons.push_back(*i);
			}
			polygons.clear();
		}
		if (circles.size() != 0) {
			for (vector<Circle>::iterator i = circles.begin(); i != circles.end(); ++i) {
				throw_circles.push_back(*i);
			}
			circles.clear();
		}

		glutPostRedisplay();//直接redisplay，不然會等到下一次的mouseclick才會執行動作
	}
	//r R
	else if (key == 114 || key || 82) {
		cout << "redraw\n";
		//cur_state = 6;  //去掉這一行，這樣按下r時只會執行恢復的動作，功能則會維持在原本的畫畫功能，不用再按一次功能選項鍵
		for (vector<Point>::iterator i = throw_points.begin(); i != throw_points.end(); i++) {
			points.push_back(*i);
		}
		for (vector<Line>::iterator i = throw_lines.begin(); i != throw_lines.end(); i++) {
			lines.push_back(*i);
		}
		for (vector<vector<Line>>::iterator i = throw_polygons.begin(); i != throw_polygons.end(); ++i) {
			polygons.push_back(*i);
		}
		for (vector<Circle>::iterator i = throw_circles.begin(); i != throw_circles.end(); ++i) {
			circles.push_back(*i);
		}
		glutPostRedisplay();//直接redisplay，不燃會等到下一次的mouseclick才會執行動作
	}
}
void MyMouse(int button, int state, int x, int y) {
	if (cur_state == 1) {//draw dot
		if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
			//xcur = x;
			//ycur = height-y;
			cout << x << "," << y << endl;
			Point p;
			p.x = x;
			p.y = height - y;
			points.push_back(p);
		}
	}
	else if (cur_state == 2) {
		if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
			if (linefirstp == 1) {//the first point was done
				cout << "linefirstp:" << linefirstp << endl;
				linefirstp = 0;//linefirst==2--> second point checked, to the drawing state
				Point p;
				p.x = x;
				p.y = height - y;
				//linepoint2 = p;/////////////////////
				//starts forming line
				Line line;
				line.p2 = p;
				line.p1 = linepoint1;
				lines.push_back(line);
				cout << "linepoint2:" << p.x << " " << p.y << endl;
			}
			else {//linefirst==0, no line yet
				cout << "linefirstp:" << linefirstp << endl;
				linefirstp++; //==1
				Point p;
				p.x = x;
				p.y = height - y;
				linepoint1 = p;
				cout << "linepoint1:" << p.x << " " << p.y << endl;
			}
		}
	}
	else if (cur_state == 3) {
		if (polystart != true) {//starts polygon's process
			if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
				polystart = true; // this is the start of polygon
				Point p;
				p.x = x;
				p.y = y;
				polylinefirstpoint = p;
				polypoint1 = p; //the last point should automatically get back to this point
				polygons.push_back(curpolyline);
			}
		}
		else {
			if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
				Point p;
				p.x = x;
				p.y = y;
				Line l;
				l.p1 = polylinefirstpoint;
				l.p2 = p;
				curpolyline.push_back(l);
				polylinefirstpoint = p;//the 2nd point turns into the 1st point of the next line
				polygons.pop_back();
				polygons.push_back(curpolyline);
			}
			else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
				Line l;
				l.p1 = polylinefirstpoint;
				l.p2 = polypoint1;
				curpolyline.push_back(l);
				polystart = false;
				polygons.pop_back();
				polygons.push_back(curpolyline);//push all the lines to a polygon
				curpolyline.clear();
			}
		}
	}
	else if (cur_state == 4) {
		if (circlestart != true) {
			if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
				circlestart = true;
				Point p;
				p.x = x;
				p.y = height - y;
				circlefirstpoint = p;
				//cout <<"1: "<< x << ", " << y << endl;
			}
		}
		else {
			if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
				Point p;
				p.x = x;
				p.y = height - y;
				//cout << "2: " << x << ", " << y << endl;
				float r = sqrt(pow(circlefirstpoint.x - p.x, 2) + pow(circlefirstpoint.y - p.y, 2));
				Circle c;
				c.center = circlefirstpoint;
				c.radius = r;
				circles.push_back(c);
				circlestart = false;

				glutPostRedisplay();
			}
		}
	}
}
void Initial(void)//initialize函數
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);//glClearColor():設置背景顏色，(r,g,b,alpha):0,0,0->黑，1,1,1->白
	glMatrixMode(GL_PROJECTION); //OpenGL按照三维方式來處理圖像，"PROJECTION"--一個投影變換將三维圖形投影到顯示器的二维空間中
	gluOrtho2D(0.0, width, 0.0, height);//指定使用正投影將一個x座標在0~800，y座標0~600範圍內的矩形座標區域 投影到顯示器的窗口

}
void myDisplay(void)//display回調函數
{
	//glClear():用來清除屏幕顏色 = 將屏幕的所有像素點都還原為“底色 ”
	glClear(GL_COLOR_BUFFER_BIT);//使用glClearColorz中指定的值 來設定顏色緩存區buffer的值，
								 //即將窗口中的每一個像素設置為背景色。
								 //“GL_COLOR_BUFFER_BIT”--指定當前被激活為寫操作的顏色緩存
	//在OpenGl中設置顏色，一般可以使用glColor3f()
	//glColor3f(1.0f, 1.0f, 1.0f);//繪圖顏色為白色，(r,g,b)
	//glRectf(50.0f, 50.0f, 750.0f, 550.0f);//圖形的座標，繪製一個左上角在（50，50），右下角在（750，550）的矩形
	glPointSize(2.5);
	glBegin(GL_POINTS);
	glColor3f(1.0f, 1.0f, 1.0f);//繪圖顏色為白色，(r,g,b)
	if (points.size() != 0) {
		for (vector<Point>::iterator i = points.begin(); i != points.end(); ++i) {
			glVertex2d((*i).x, (*i).y);
		}
		//cout << "draw dot\n";
		//glVertex2d(xcur, ycur);
	}
	glEnd();
	glPointSize(1.0f);
	glBegin(GL_POINTS);
	glColor3f(0.5f, 0.5f, 1.0f);
	//glPointSize(1.5f);<--沒效
	if (lines.size() != 0) {
		for (vector<Line>::iterator i = lines.begin(); i != lines.end(); ++i) {
			cout << "linefirstp:" << linefirstp << " drawing start" << endl;
			cout << "point 1:" << (*i).p1.x << " " << (*i).p1.y << endl;
			cout << "point 2:" << (*i).p2.x << " " << (*i).p2.y << endl;

			drawline((*i).p1.x, (*i).p1.y, (*i).p2.x, (*i).p2.y);
			cout << "draw line end\n";
			//linefirstp = 0;
		}
	}
	glEnd();
	glPointSize(1.0f);
	glBegin(GL_POINTS);
	glColor3f(1.0f, 0.98f, 0.65f);
	if (polygons.size() != 0) {
		for (vector<vector<Line>>::iterator i = polygons.begin(); i != polygons.end(); ++i) {
			for (vector<Line>::iterator j = (*i).begin(); j != (*i).end(); ++j) {
				drawline((*j).p1.x, height - (*j).p1.y, (*j).p2.x, height - (*j).p2.y);
				//cout << "draw line end\n";
			}
			cout << "draw one polygon\n";
		}
	}
	glEnd();
	glBegin(GL_POINTS);
	glColor3f(0.69f, 0.87f, 0.9f);
	if (circles.size() != 0) {
		for (vector<Circle>::iterator i = circles.begin(); i != circles.end(); ++i) {
			cout << "center: (" << (*i).center.x << ", " << (*i).center.y << ")\nradius: " << (*i).radius << endl;
			//c_state = 1;
			drawcircle((*i).radius, (*i).center);
			cout << "draw a circle\n";
		}
	}
	glEnd();

	//glEnd();
	glFlush();//清空OpenGL命令緩衝區，強制執行命令緩衝區中所有OpenGL函數
}
int main(int argc, char * argv[])//使用glut庫函數進行窗口管理
{
	glutInit(&argc, argv);//使用glut庫需要進行初始化
	glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);//設定窗口顯示模式 : (顏色|緩存)，這裡是RGB顏色模型和單緩存
	glutInitWindowPosition(100, 100);//設定窗口的初始位置，屏幕左上角為原點，單位為像素
	glutInitWindowSize(width, height);//設定窗口大小
	glutCreateWindow("小畫家");//創建一個窗口。glutCreateWindow ("窗口標題名")
	Initial();
	glutDisplayFunc(&myDisplay);//將myDisplay指定為當前窗口的顯示内容函數
	glutKeyboardFunc(&MyKeyboard);
	glutMouseFunc(&MyMouse);

	glutMainLoop(); //使窗口框架運行起來，使顯示回調函數(void myDisplay())開始工作
	return 0;
}