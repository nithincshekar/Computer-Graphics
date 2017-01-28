/* **************************
* CSCI 420
* Assignment 3 Raytracer
* Name: Nithin Chandrashekhar
* *************************
*/

#ifdef WIN32
#include <windows.h>
#endif

#if defined(WIN32) || defined(linux)
#include <GL/gl.h>
#include <GL/glut.h>
#elif defined(__APPLE__)
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#define strcasecmp _stricmp
#endif

#include <imageIO.h>
#include <algorithm>

// PI value 22/7
#define PI 3.14

#define MAX_TRIANGLES 20000
#define MAX_SPHERES 100
#define MAX_LIGHTS 100

char * filename = NULL;

//different display modes
#define MODE_DISPLAY 1
#define MODE_JPEG 2

int mode = MODE_DISPLAY;

//you may want to make these smaller for debugging purposes
#define WIDTH 640
#define HEIGHT 480

// image pixel values
double img_width;
double img_height;
double img_left;
double img_bottom;

double min_val1 = 0.0000001f;
double min_val2 = 0.000001f;

bool ANTIALIAS_FLAG = true;

//the field of view of the camera
#define fov 60.0

unsigned char buffer[HEIGHT][WIDTH][3];

struct Point
{
	double x;
	double y;
	double z;
};

struct Vertex
{
	double position[3];
	double color_diffuse[3];
	double color_specular[3];
	double normal[3];
	double shininess;
};

struct Triangle
{
	Vertex v[3];
};

struct Sphere
{
	double position[3];
	double color_diffuse[3];
	double color_specular[3];
	double shininess;
	double radius;
};

struct Light
{
	double position[3];
	double color[3];
};

Triangle triangles[MAX_TRIANGLES];
Sphere spheres[MAX_SPHERES];
Light lights[MAX_LIGHTS];
double ambient_light[3];

int num_triangles = 0;
int num_spheres = 0;
int num_lights = 0;

void plot_pixel_display(int x, int y, unsigned char r, unsigned char g, unsigned char b);
void plot_pixel_jpeg(int x, int y, unsigned char r, unsigned char g, unsigned char b);
void plot_pixel(int x, int y, unsigned char r, unsigned char g, unsigned char b);

void plot_pixel_display(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
	glColor3f(((float)r) / 255.0f, ((float)g) / 255.0f, ((float)b) / 255.0f);
	glVertex2i(x, y);
}

void plot_pixel_jpeg(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
	buffer[HEIGHT - y - 1][x][0] = r;
	buffer[HEIGHT - y - 1][x][1] = g;
	buffer[HEIGHT - y - 1][x][2] = b;
}

void plot_pixel(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
	plot_pixel_display(x, y, r, g, b);
	if (mode == MODE_JPEG)
		plot_pixel_jpeg(x, y, r, g, b);
}

void save_jpg()
{
	printf("Saving JPEG file: %s\n", filename);

	ImageIO img(WIDTH, HEIGHT, 3, &buffer[0][0][0]);
	img.flipVertically();
	if (img.save(filename, ImageIO::FORMAT_JPEG) != ImageIO::OK)
		printf("Error in Saving\n");
	else
		printf("File saved Successfully\n");
}

void parse_check(const char *expected, char *found)
{
	if (strcasecmp(expected, found))
	{
		printf("Expected '%s ' found '%s '\n", expected, found);
		printf("Parse error, abnormal abortion\n");
		exit(0);
	}
}

void parse_doubles(FILE* file, const char *check, double p[3])
{
	char str[100];
	fscanf(file, "%s", str);
	parse_check(check, str);
	fscanf(file, "%lf %lf %lf", &p[0], &p[1], &p[2]);
	printf("%s %lf %lf %lf\n", check, p[0], p[1], p[2]);
}

void parse_rad(FILE *file, double *r)
{
	char str[100];
	fscanf(file, "%s", str);
	parse_check("rad:", str);
	fscanf(file, "%lf", r);
	printf("rad: %f\n", *r);
}

void parse_shi(FILE *file, double *shi)
{
	char s[100];
	fscanf(file, "%s", s);
	parse_check("shi:", s);
	fscanf(file, "%lf", shi);
	printf("shi: %f\n", *shi);
}

int loadScene(char *argv)
{
	FILE * file = fopen(argv, "r");
	int number_of_objects;
	char type[50];
	Triangle t;
	Sphere s;
	Light l;
	fscanf(file, "%i", &number_of_objects);

	printf("number of objects: %i\n", number_of_objects);

	parse_doubles(file, "amb:", ambient_light);

	for (int i = 0; i<number_of_objects; i++)
	{
		fscanf(file, "%s\n", type);
		printf("%s\n", type);
		if (strcasecmp(type, "triangle") == 0)
		{
			printf("found triangle\n");
			for (int j = 0; j < 3; j++)
			{
				parse_doubles(file, "pos:", t.v[j].position);
				parse_doubles(file, "nor:", t.v[j].normal);
				parse_doubles(file, "dif:", t.v[j].color_diffuse);
				parse_doubles(file, "spe:", t.v[j].color_specular);
				parse_shi(file, &t.v[j].shininess);
			}

			if (num_triangles == MAX_TRIANGLES)
			{
				printf("too many triangles, you should increase MAX_TRIANGLES!\n");
				exit(0);
			}
			triangles[num_triangles++] = t;
		}
		else if (strcasecmp(type, "sphere") == 0)
		{
			printf("found sphere\n");

			parse_doubles(file, "pos:", s.position);
			parse_rad(file, &s.radius);
			parse_doubles(file, "dif:", s.color_diffuse);
			parse_doubles(file, "spe:", s.color_specular);
			parse_shi(file, &s.shininess);

			if (num_spheres == MAX_SPHERES)
			{
				printf("too many spheres, you should increase MAX_SPHERES!\n");
				exit(0);
			}
			spheres[num_spheres++] = s;
		}
		else if (strcasecmp(type, "light") == 0)
		{
			printf("found light\n");
			parse_doubles(file, "pos:", l.position);
			parse_doubles(file, "col:", l.color);

			if (num_lights == MAX_LIGHTS)
			{
				printf("too many lights, you should increase MAX_LIGHTS!\n");
				exit(0);
			}
			lights[num_lights++] = l;
		}
		else
		{
			printf("unknown type in scene description:\n%s\n", type);
			exit(0);
		}
	}
	return 0;
}


// normalize the given point
Point unitize(Point ip)
{
	Point res;
	// calculate the magnitude
	double magnitude = std::sqrt(std::pow(ip.x, 2) + std::pow(ip.y, 2) + std::pow(ip.z, 2));
	if (magnitude == 0.0)
		magnitude = 0.0001;
	// divide each co-ordinate by the magnitude
	res.x = ip.x / magnitude;
	res.y = ip.y / magnitude;
	res.z = ip.z / magnitude;
	return res;
}

// vector subtraction given two inputs
Point vec_subtraction(Point a, Point b)
{
	Point res;
	res.x = a.x - b.x;
	res.y = a.y - b.y;
	res.z = a.z - b.z;
	return res;
}

// cross product given two inputs
Point cross_product(Point pt1, Point pt2) {
	Point res;
	res.x = pt1.y * pt2.z - pt2.y * pt1.z;
	res.y = pt2.x * pt1.z - pt1.x * pt2.z;
	res.z = pt1.x * pt2.y - pt1.y * pt2.x;
	return res;
}

// calculate the area  of the triangel given three points
double tri_area(Point a, Point b, Point c)
{
	Point b_a, c_a, area;
	double res;
	b_a = vec_subtraction(b, a);
	c_a = vec_subtraction(c, a);
	area = cross_product(b_a, c_a);
	res = sqrt(std::pow(area.x, 2) + std::pow(area.y, 2) + std::pow(area.z, 2)) / 2;
	return res;
}

// dot product given two inputs
double dot_product(Point a, Point b)
{
	double res;
	res = a.x * b.x + a.y * b.y + a.z * b.z;
	return res;
}

// function for calculating the hit based on the given input
Point cal_hit(double t, Point translation, Point res_dirtn)
{
	Point hit;
	hit.x = translation.x + t * res_dirtn.x;
	hit.y = translation.y + t * res_dirtn.y;
	hit.z = translation.z + t * res_dirtn.z;
	return hit;
}

// calculate the intersection between the ray and the triangle
Triangle* tri_intersection(Point direction, double * out, Point translation) 
{
	Point dirtn, res_dirtn;
	dirtn.x = direction.x - translation.x;
	dirtn.y = direction.y - translation.y;
	dirtn.z = direction.z - translation.z;
	res_dirtn = unitize(dirtn);
	Triangle * res_triangle = NULL;
	
	// iterate through all the given triangles
	for (int x = 0; x < num_triangles; x++)
	{
		// calculation done as per the formulas given in the slides
		Point tmp, n;
		Point a, b, c;
		a.x = triangles[x].v[1].position[0];
		a.y = triangles[x].v[1].position[1];
		a.z = triangles[x].v[1].position[2];
		b.x = triangles[x].v[0].position[0];
		b.y = triangles[x].v[0].position[1];
		b.z = triangles[x].v[0].position[2];
		c.x = triangles[x].v[2].position[0];
		c.y = triangles[x].v[2].position[1];
		c.z = triangles[x].v[2].position[2];

		Point a_b = vec_subtraction(a, b);
		Point c_b = vec_subtraction(c, b);

		// calculate the normal
		tmp = cross_product(a_b, c_b);
		n = unitize(tmp);

		// calculate the dot product
		double nd_dot = dot_product(n, res_dirtn);
		{
			Point op;
			op.x = translation.x - triangles[x].v[0].position[0];
			op.y = translation.y - triangles[x].v[0].position[1];
			op.z = translation.z - triangles[x].v[0].position[2];
			double t = -dot_product(op, n) / nd_dot;
			if (t > (-1 * min_val1) && t < min_val1)
				t = 0.0f;
			if (t>0.f)
			{
				Point hit;
				Point p1, p2, p3;
				p1.x = triangles[x].v[2].position[0];
				p1.y = triangles[x].v[2].position[1];
				p1.z = triangles[x].v[2].position[2];
				p2.x = triangles[x].v[1].position[0];
				p2.y = triangles[x].v[1].position[1];
				p2.z = triangles[x].v[1].position[2];
				p3.x = triangles[x].v[0].position[0];
				p3.y = triangles[x].v[0].position[1];
				p3.z = triangles[x].v[0].position[2];

				// calculate the hit
				hit = cal_hit(t, translation, res_dirtn);

				Point p2_p1 = vec_subtraction(p1, p2);
				Point p0_p2 = vec_subtraction(p3, p1);
				Point hit_p1 = vec_subtraction(hit, p3);
				Point hit_p2 = vec_subtraction(hit, p2);
				Point hit_p3 = vec_subtraction(hit, p1);

				Point cross_1 = cross_product(a_b, hit_p1);
				Point cross_2 = cross_product(p2_p1, hit_p2);
				Point cross_3 = cross_product(p0_p2, hit_p3);

				double d1 = dot_product(cross_1, n);
				double d2 = dot_product(cross_2, n);
				double d3 = dot_product(cross_3, n);

				if (0 <= d1 && 0 <= d2 && 0 <= d3)
				{
					double dbl_val = *out;
					if (t < dbl_val)
					{
						*out = t;
						res_triangle = &triangles[x];
					}
				}
			}
		}
	}
	return res_triangle;
}

// calculate the intersection between the ray and the sphere
Sphere* sph_intersection(Point direction, double * out, Point translation)
{
	Point dirtn, res_dirtn;
	dirtn.x = direction.x - translation.x;
	dirtn.y = direction.y - translation.y;
	dirtn.z = direction.z - translation.z;
	res_dirtn = unitize(dirtn);
	Sphere * res_sphere = NULL;
	// iterate through all the given spheres
	for (int x = 0; x < num_spheres; x++)
	{
		// calculation done as per the formulas given in the slides
		double c = (translation.x - spheres[x].position[0])*(translation.x - spheres[x].position[0]) +
			(translation.y - spheres[x].position[1])*(translation.y - spheres[x].position[1]) +
			(translation.z - spheres[x].position[2])*(translation.z - spheres[x].position[2]) -
			spheres[x].radius*spheres[x].radius;
		double b = 2 * (res_dirtn.x * (translation.x - spheres[x].position[0]) +
			res_dirtn.y * (translation.y - spheres[x].position[1]) + res_dirtn.z *
			(translation.z - spheres[x].position[2]));

		double in_val = pow(b, 2) - 4 * c;
		// if in_val is not negative
		if (in_val >= 0)
		{
			double t1 = (-b - sqrt(in_val)) / 2;
			double t0 = (-b + sqrt(in_val)) / 2;
			if (t0 > -0.0001f && t0 <= 0.0001f)
				t0 = 0.0f;
			if (t1 > -0.0001f && t1 < 0.0001f)
				t1 = 0.0f;
			if (t0 > 0.f && t0 < *out)
			{
				*out = t0;
				res_sphere = &spheres[x];
			}
			if (t1 > 0.f && t1 < *out)
			{
				*out = t1;
				res_sphere = &spheres[x];
			}
		}
	}
	return res_sphere;
}

bool shadow_check(Light * dst, Point src);

// function for debugging
Point cal_one(Point ray_point, Sphere *hit_sphere)
{
	Point tmp1, normal;
	tmp1.x = ray_point.x - hit_sphere->position[0];
	tmp1.y = ray_point.y - hit_sphere->position[1];
	tmp1.z = ray_point.z - hit_sphere->position[2];
	normal = unitize(tmp1);
	return normal;
}

// function for debugging
Point cal_two(Point ray_point)
{
	Point tmp2, view_vector;
	tmp2.x = -ray_point.x;
	tmp2.y = -ray_point.y;
	tmp2.z = -ray_point.z;
	view_vector = unitize(tmp2);
	return view_vector;
}

// function for debugging
Point cal_three(Point ray_point)
{
	Point tmp3;
	tmp3.x = lights->position[0] - ray_point.x;
	tmp3.y = lights->position[1] - ray_point.y;
	tmp3.z = lights->position[2] - ray_point.z;
	return unitize(tmp3);
}

// function for debugging
Point cal_four(Point light_vector, Point normal, double LxN)
{
	Point tmp4, reflected_vector;
	tmp4.x = 2 * LxN * normal.x - light_vector.x;
	tmp4.y = 2 * LxN * normal.y - light_vector.y;
	tmp4.z = 2 * LxN * normal.z - light_vector.z;
	reflected_vector = unitize(tmp4);
	return reflected_vector;
}

// function for calculating the normal
Point cal_normal(double P0value, double P1value, double P2value, Triangle *hit_tri)
{
	Point tmp, normal;
	tmp.x = P0value * hit_tri->v[0].normal[0] + P1value * hit_tri->v[1].normal[0] + P2value * hit_tri->v[2].normal[0];
	tmp.y = P0value * hit_tri->v[0].normal[1] + P1value * hit_tri->v[1].normal[1] + P2value * hit_tri->v[2].normal[1];
	tmp.z = P0value * hit_tri->v[0].normal[2] + P1value * hit_tri->v[1].normal[2] + P2value * hit_tri->v[2].normal[2];

	normal = unitize(tmp);
	return normal;
}

// calculate the diffuse color
Point cal_dc(double P0value, double P1value, double P2value, Triangle *hit_tri)
{
	Point dif_color;
	dif_color.x = P0value * hit_tri->v[0].color_diffuse[0] + P1value * hit_tri->v[1].color_diffuse[0] + P2value * hit_tri->v[2].color_diffuse[0];
	dif_color.y = P0value * hit_tri->v[0].color_diffuse[1] + P1value * hit_tri->v[1].color_diffuse[1] + P2value * hit_tri->v[2].color_diffuse[1];
	dif_color.z = P0value * hit_tri->v[0].color_diffuse[2] + P1value * hit_tri->v[1].color_diffuse[2] + P2value * hit_tri->v[2].color_diffuse[2];
	return dif_color;
}

// calculate the specular color
Point cal_sc(double P0value, double P1value, double P2value, Triangle *hit_tri)
{
	Point spec_color;
	spec_color.x = P0value * hit_tri->v[0].color_specular[0] + P1value * hit_tri->v[1].color_specular[0] + P2value * hit_tri->v[2].color_specular[0];
	spec_color.y = P0value * hit_tri->v[0].color_specular[1] + P1value * hit_tri->v[1].color_specular[1] + P2value * hit_tri->v[2].color_specular[1];
	spec_color.z = P0value * hit_tri->v[0].color_specular[2] + P1value * hit_tri->v[1].color_specular[2] + P2value * hit_tri->v[2].color_specular[2];
	return spec_color;
}

// calculate the color for triangles and spheres 
void cal_ray(double *color, double x, double y)
{
	Point pos_screen;
	// calculate the co-ordinates based on the image size
	pos_screen.x = img_left + img_width / 2 + x * img_width;
	pos_screen.y = img_bottom + img_height / 2 + y * img_height;
	pos_screen.z = -1;
	Point translation = { 0.0, 0.0, 0.0 };

	// sphere and triangle distance
	double sphr_dist = 400000000000;
	double tri_dist =  200000000000;

	Point ray_point;
	Point pt_ray;

	// sphere intersection
	Sphere *hit_sphere = sph_intersection(pos_screen, &sphr_dist, translation);

	// triangle intersection
	Triangle *hit_tri = tri_intersection(pos_screen, &tri_dist, translation);
	
	pt_ray = unitize(pos_screen);
	color[0] = ambient_light[0];
	color[1] = ambient_light[1];
	color[2] = ambient_light[2];

	// for sphere
	if (sphr_dist < tri_dist && hit_sphere)
	{
		ray_point.x = sphr_dist * pt_ray.x;
		ray_point.y = sphr_dist * pt_ray.y;
		ray_point.z = sphr_dist * pt_ray.z;

		for (int x = 0; x < num_lights; x++)
		{
			if (!shadow_check(&lights[x], ray_point))
			{
				Point normal;
				normal = cal_one(ray_point, hit_sphere);

				Point view_vector;
				view_vector = cal_two(ray_point);

				Point light_vector;
				light_vector = cal_three(ray_point);

				double LxN = dot_product(light_vector, normal);
				Point reflected_vector;
				reflected_vector = cal_four(light_vector, normal, LxN);

				double RxV = dot_product(reflected_vector, view_vector);
				if (LxN < 0)
					LxN = 0;
				if (RxV < 0)
					RxV = 0;

				color[0] += lights->color[0] * (hit_sphere->color_diffuse[0] * (LxN)+hit_sphere->color_specular[0] * pow(RxV, hit_sphere->shininess));
				color[1] += lights->color[0] * (hit_sphere->color_diffuse[1] * (LxN)+hit_sphere->color_specular[1] * pow(RxV, hit_sphere->shininess));
				color[2] += lights->color[0] * (hit_sphere->color_diffuse[2] * (LxN)+hit_sphere->color_specular[2] * pow(RxV, hit_sphere->shininess));
			}
		}
	}
	// for triangle
	else if (hit_tri)
	{
		ray_point.x = tri_dist * pt_ray.x;
		ray_point.y = tri_dist * pt_ray.y;
		ray_point.z = tri_dist * pt_ray.z;

		for (int x = 0; x < num_lights; x++)
		{
			if (!shadow_check(&lights[x], ray_point))
			{
				Point pt_Diffuse_Color, pt_Specular_Color;
				double dif_color[3];
				double spec_color[3];
				double shininess;

				Point normal;
				Point a, b, c;
				a.x = hit_tri->v[0].position[0];
				a.y = hit_tri->v[0].position[1];
				a.z = hit_tri->v[0].position[2];
				b.x = hit_tri->v[1].position[0];
				b.y = hit_tri->v[1].position[1];
				b.z = hit_tri->v[1].position[2];
				c.x = hit_tri->v[2].position[0];
				c.y = hit_tri->v[2].position[1];
				c.z = hit_tri->v[2].position[2];
				double Area = tri_area(a, b, c);

				double P0value = tri_area(b, c, ray_point) / Area;
				double P1value = tri_area(c, a, ray_point) / Area;
				double P2value = tri_area(a, b, ray_point) / Area;

				normal = cal_normal(P0value, P1value, P2value, hit_tri);

				pt_Diffuse_Color = cal_dc(P0value, P1value, P2value, hit_tri);
				dif_color[0] = pt_Diffuse_Color.x;
				dif_color[1] = pt_Diffuse_Color.y;
				dif_color[2] = pt_Diffuse_Color.z;

				pt_Specular_Color = cal_sc(P0value, P1value, P2value, hit_tri);
				spec_color[0] = pt_Specular_Color.x;
				spec_color[1] = pt_Specular_Color.y;
				spec_color[2] = pt_Specular_Color.z;
				shininess = P0value * hit_tri->v[0].shininess + P1value * hit_tri->v[1].shininess + P2value * hit_tri->v[2].shininess;

				Point tmp1, view_vector;
				tmp1.x = -ray_point.x;
				tmp1.y = -ray_point.y;
				tmp1.z = -ray_point.z;
				view_vector = unitize(tmp1);

				Point tmp2, light_vector;
				tmp2.x = lights->position[0] - ray_point.x;
				tmp2.y = lights->position[1] - ray_point.y;
				tmp2.z = lights->position[2] - ray_point.z;
				light_vector = unitize(tmp2);

				double LxN = dot_product(light_vector, normal);
				Point tmp3, reflected_vector;
				tmp3.x = 2 * LxN * normal.x - light_vector.x;
				tmp3.y = 2 * LxN * normal.y - light_vector.y;
				tmp3.z = 2 * LxN * normal.z - light_vector.z;
				reflected_vector = unitize(tmp3);

				double RxV = dot_product(reflected_vector, view_vector);
				if (LxN < 0)
					LxN = 0;
				if (RxV < 0)
					RxV = 0;

				color[0] += lights->color[0] * (dif_color[0] * (LxN)+spec_color[0] * pow(RxV, shininess));
				color[1] += lights->color[1] * (dif_color[1] * (LxN)+spec_color[1] * pow(RxV, shininess));
				color[2] += lights->color[2] * (dif_color[2] * (LxN)+spec_color[2] * pow(RxV, shininess));
			}
		}
	}
	else
	{
		color[0] = 1.0; color[1] = 1.0; color[2] = 1.0;
	}
}

// check for the shadows
bool shadow_check(Light * dst, Point src)
{
	double dist = sqrt(pow((dst->position[0] - src.x), 2)
		+ pow((dst->position[1] - src.y), 2) +
		pow((dst->position[2] - src.z), 2));
	double light_tri_distance = dist;

	Point light_pos;
	light_pos.x = dst->position[0];
	light_pos.y = dst->position[1];
	light_pos.z = dst->position[2];

	if (sph_intersection(light_pos, &dist, src))
		return true;
	if (tri_intersection(light_pos, &light_tri_distance, src))
		return true;
	return false;
}

// find color using four neighbouring pixels for anti aliasing
void anti_aliasing(int x, int y)
{
	double clr[3];
	if (ANTIALIAS_FLAG)
	{
		// taking neighbouring pixel colors
		double clr1[3];
		cal_ray(clr1, x + .25f, y + .5f);
		double clr2[3];
		cal_ray(clr2, x + .5f, y + .75f);
		double clr3[3];
		cal_ray(clr3, x + .5f, y + .25f);
		double clr4[3];
		cal_ray(clr4, x + .75f, y + .5f);

		// take the average of the neighbouring pixel colors
		clr[0] = (clr1[0] + clr2[0] + clr3[0] + clr4[0]) / 4;
		if (clr[0] > 1)
			clr[0] = 1;
		else if (clr[0] < 0)
			clr[0] = 0;
		clr[0] *= 255;

		clr[1] = (clr1[1] + clr2[1] + clr3[1] + clr4[1]) / 4;
		if (clr[1] > 1)
			clr[1] = 1;
		else if (clr[1] < 0)
			clr[1] = 0;
		clr[1] *= 255;

		clr[2] = (clr1[2] + clr2[2] + clr3[2] + clr4[2]) / 4;
		if (clr[2] > 1)
			clr[2] = 1;
		else if (clr[2] < 0)
			clr[2] = 0;
		clr[2] *= 255;
	}
	else
	{
		cal_ray(clr, x, y);
		if (clr[0] > 1)
			clr[0] = 1;
		else if (clr[0] < 0)
			clr[0] = 0;
		clr[0] *= 255;
		if (clr[1] > 1)
			clr[1] = 1;
		else if (clr[1] < 0)
			clr[1] = 0;
		clr[1] *= 255;
		if (clr[2] > 1)
			clr[2] = 1;
		else if (clr[2] < 0)
			clr[2] = 0;
		clr[2] *= 255;
	}
	plot_pixel_jpeg(x, y, clr[0], clr[1], clr[2]);

}

// calculate the image corner values based on the fov
void calculateCorners()
{
	double ar = (double)WIDTH / (double)HEIGHT;
	double right = ar * tan(fov / 2 * PI / 180);
	double left = -ar * tan(fov / 2 * PI / 180);
	double bottom = -tan(fov / 2 * PI / 180);
	double top = tan((fov / 2)*(PI / 180));
	
	img_bottom = bottom;
	img_left = left;
	img_height = (top - bottom) / HEIGHT;
	img_width = (right - left) / WIDTH;
}

// not drawing anything in display as we do not need to keep drawing
void display()
{

}

void draw_scene()
{
	glutSwapBuffers();
	for (int x = 0; x < WIDTH; x++)
	{
		glPointSize(2.0);
		glBegin(GL_POINTS);
		for (int y = 0; y < HEIGHT; y++)
		{
			// anti alias to calculate colors before drawing
			anti_aliasing(x, y);
			plot_pixel(x, y, buffer[HEIGHT - y - 1][x][0], buffer[HEIGHT - y - 1][x][1], buffer[HEIGHT - y - 1][x][2]);
		}
		glEnd();
		glFlush();
	}
}

void init()
{
	glMatrixMode(GL_PROJECTION);
	glOrtho(0, WIDTH, 0, HEIGHT, 1, -1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	calculateCorners();
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
}

void idle()
{
	//hack to make it only draw once
	static int once = 0;
	if (!once)
	{
		draw_scene();
		if (mode == MODE_JPEG)
			save_jpg();
	}
	once = 1;
}

int main(int argc, char ** argv)
{
	if ((argc < 2) || (argc > 3))
	{
		printf("Usage: %s <input scenefile> [output jpegname]\n", argv[0]);
		exit(0);
	}
	if (argc == 3)
	{
		mode = MODE_JPEG;
		filename = argv[2];
	}
	else if (argc == 2)
		mode = MODE_DISPLAY;

	glutInit(&argc, argv);
	loadScene(argv[1]);

	glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(WIDTH, HEIGHT);
	int window = glutCreateWindow("Ray Tracer");
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	init();
	glutMainLoop();
}
