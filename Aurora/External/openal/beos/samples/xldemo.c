/* xldemo.c */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <AL/al.h>
#include <AL/alut.h>


static const ALbyte *kRadarFileName = "data/radar.wav";
static const ALbyte *kPhaserFileName = "data/phaser.wav";


/* camera/listener */
static float cameraPosition[3], cameraOrientation[6], cameraAngle;

/* radar and phaser */
static float radarPosition[3], phaserPosition[3], radarLightAngle;
static ALuint radarSource, phaserSource;
static ALuint radarBuffer, phaserBuffer;
static ALboolean phaserPlaying;


static void placeCamera(void)
{
	float vector[3];

	/* set up view projection */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-0.13, 0.13, -0.1, 0.1, 0.2, 50.0);

	
	/* set up the camera */
	gluLookAt(
		cameraPosition[0],
		cameraPosition[1],
		cameraPosition[2],
		
		cameraPosition[0] + cameraOrientation[0],
		cameraPosition[1] + cameraOrientation[1],
		cameraPosition[2] + cameraOrientation[2],
		
		cameraOrientation[3],
		cameraOrientation[4],
		cameraOrientation[5]);

	
	/* set up the listener */
	alListenerfv(AL_POSITION, cameraPosition);
	alListenerfv(AL_ORIENTATION, cameraOrientation);
	
	
	/* fire the phaser if the camera is close to the radar */
	vector[0] = cameraPosition[0] - radarPosition[0];
	vector[1] = cameraPosition[1] - radarPosition[1];
	vector[2] = cameraPosition[2] - radarPosition[2];

	if (vector[0] * vector[0] + vector[1] * vector[1] + vector[2] * vector[2] <= 9.0f) {
		if (phaserPlaying == AL_FALSE) {
			alSourcePlay(phaserSource);
			phaserPlaying = AL_TRUE;
		}
	}
	else {
		if (phaserPlaying == AL_TRUE) {
			alSourceStop(phaserSource);
			phaserPlaying = AL_FALSE;
		}
	}
  
}

static void translateCamera(float distance)
{
	cameraPosition[0] += distance * cameraOrientation[0];
	cameraPosition[1] += distance * cameraOrientation[1];
	cameraPosition[2] += distance * cameraOrientation[2];
	
	placeCamera();
}

static void stepCamera(float distance)
{
	cameraPosition[0] += distance * cos(2.0 * M_PI * (cameraAngle + 90) / 360.0);
	cameraPosition[2] += distance * sin(2.0 * M_PI * (cameraAngle + 90) / 360.0);
	
	placeCamera();
}

static void rotateCamera(float angle)
{
	cameraAngle += angle;
	
	cameraOrientation[0] = cos(2.0 * M_PI * cameraAngle / 360.0);
	cameraOrientation[2] = sin(2.0 * M_PI * cameraAngle / 360.0);
	
	placeCamera();
}


static void drawHouse(void)
{
	/* Background */
	glClearColor( 0.6f, 0.6f, 1.0f, 0.0f );
	glClear( GL_COLOR_BUFFER_BIT );
	
	/* Sky */
	glPushMatrix();
	glBegin (GL_QUADS);
	glColor3f( 0.0f, 0.0f, 1.0f );
	glNormal3f (0.0f, 1.0f, 0.0f);
	glVertex3f (-50.0f, 6.0f, -50.0f);
	glVertex3f (50.0f, 6.0f, -50.0f);
	glVertex3f (50.0f, 6.0f, 50.0f);
	glVertex3f (-50.0f, 6.0f, 50.0f);
	glEnd();
	glPopMatrix();
	
	/* Ground */
	glPushMatrix();
	glBegin (GL_QUADS);
	glColor3f( 0.5f, 0.5f, 0.0f );
	glNormal3f (0.0f, 1.0f, 0.0f);
	glVertex3f (-50.0f, 0.0f, -50.0f);
	glVertex3f (-50.0f, 0.0f, 50.0f);
	glVertex3f (50.0f, 0.0f, 50.0f);
	glVertex3f (50.0f, 0.0f, -50.0f);
	glEnd();
	glPopMatrix();
	
	/* Rotating triangle */
	glPushMatrix();
	glTranslatef(radarPosition[0] + 0.5f, radarPosition[1], radarPosition[2] - 0.5f);
	glRotatef( radarLightAngle, 0.0f, 1.0f, 0.0f );
	glBegin( GL_TRIANGLES );
	glColor4f( 0.0f, 1.0f, 0.0f, 0.5f );
	glVertex3f( 0.0f, 1.25f, 0.0f );
	glVertex3f( 4.0f, 1.25f, 0.0f );
	glVertex3f( 2.0f, 1.25f, 4.0f);
	glEnd();
	glPopMatrix();
	
	
	/* House */
	glPushMatrix();
	
	glTranslatef(radarPosition[0], radarPosition[1], radarPosition[2]);
	
	glBegin(GL_QUADS); /* wall #1 */
	glColor3f(0.7f, 0.7f, 0.0f);
	glNormal3f (0.0f, 0.0f, 1.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(1.0f, 0.0f, 0.0f);
	glVertex3f(1.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);
	glEnd();
	
	glBegin(GL_QUADS); /* wall #2 */
	glColor3f(0.7f, 0.7f, 0.0f);
	glNormal3f (1.0f, 0.0f, 0.0f);
	glVertex3f(1.0f, 0.0f, 0.0f);
	glVertex3f(1.0f, 0.0f, -1.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glVertex3f(1.0f, 1.0f, 0.0f);
	glEnd();
	
	glBegin(GL_QUADS); /* wall #3 */
	glColor3f(0.7f, 0.7f, 0.0f);
	glNormal3f (0.0f, 0.0f, -1.0f);
	glVertex3f(1.0f, 0.0f, -1.0f);
	glVertex3f(0.0f, 0.0f, -1.0f);
	glVertex3f(0.0f, 1.0f, -1.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glEnd();
	
	glBegin(GL_QUADS); /* wall #4 */
	glColor3f(0.7f, 0.7f, 0.0f);
	glNormal3f (-1.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, -1.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, 1.0f, -1.0f);
	glEnd();
	
	glBegin(GL_TRIANGLES); /* upper extension of wall #1 */
	glColor3f(0.7f, 0.7f, 0.0f);
	glVertex3f (0.0f, 1.0f, 0.0f);
	glVertex3f (1.0f, 1.0f, 0.0f);
	glVertex3f (0.5f, 1.25f, 0.0f);
	glEnd();
	
	glBegin(GL_TRIANGLES); /* upper extension of wall #3 */
	glColor3f(0.7f, 0.7f, 0.0f);
	glVertex3f (1.0f, 1.0f, -1.0f);
	glVertex3f (0.0f, 1.0f, -1.0f);
	glVertex3f (0.5f, 1.25f, -1.0f);
	glEnd();
	
	glBegin(GL_QUADS); /* roof section */
	glColor3f(0.9f, 0.9f, 0.9f);
	glNormal3f(-0.25f, 0.5f, 0.0f); /* would have to normalize for lighting */
	glVertex3f(0.0f, 1.0f, -1.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.5f, 1.25f, 0.0f);
	glVertex3f(0.5f, 1.25f, -1.0f);
	glEnd();
	
	glBegin(GL_QUADS); /* roof section */
	glColor3f(0.9f, 0.9f, 0.9f);
	glNormal3f(0.25f, 0.5f, 0.0f); /* would have to normalize for lighting */
	glVertex3f(1.0f, 1.0f, 0.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glVertex3f(0.5f, 1.25f, -1.0f);
	glVertex3f(0.5f, 1.25f, 0.0f);
	glEnd();
	
	glPopMatrix();
}



static void display(void)
{
	placeCamera();
	drawHouse();
	glutSwapBuffers();
}


static void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 27:
		exit(0);
		break;
	case 'z':
		/* move to the left */
		stepCamera(-0.15f);
		break;
	case 'x':
		/* move to the right */
		stepCamera(0.15f);
		break;
	}
}

static void special(int key, int x, int y)
{
	switch (key) {
	case GLUT_KEY_UP:
		/* move forward */
		translateCamera(0.15f);
		break;
	case GLUT_KEY_DOWN:
		/* move backward */
		translateCamera(-0.15f);
		break;
	case GLUT_KEY_RIGHT:
		/* rotate to the right */
		rotateCamera(5.0f);
		break;
	case GLUT_KEY_LEFT:
		/* rotate to the left */
		rotateCamera(-5.0f);
		break;
	}
}

static void reshape(int width, int height)
{
	glViewport(0, 0, width, height);
}

static void idle(void)
{
	/* update the radar light angle */
	radarLightAngle += 7.0f;
	glutPostRedisplay();
}



int main(int argc, char *argv[])
{
	ALenum format;
	ALsizei size, freq;
	ALvoid *data;


	/* setup camera position and orientation */
	cameraAngle = 0.0f;
	
	cameraPosition[0] = 0.0f;
	cameraPosition[1] = 0.8f;
	cameraPosition[2] = 0.0f;

	cameraOrientation[0] = cos(2.0 * M_PI * cameraAngle / 360.0);
	cameraOrientation[1] = 0.0f;
	cameraOrientation[2] = sin(2.0 * M_PI * cameraAngle / 360.0);
	
	cameraOrientation[3] = 0.0f;
	cameraOrientation[4] = 1.0f;
	cameraOrientation[5] = 0.0f;


	/* setup radar and phase position */
	radarPosition[0] = 5.0f;
	radarPosition[1] = 0.0f;
	radarPosition[2] = 0.0f;
	
	phaserPosition[0] = 2.0f;
	phaserPosition[1] = 0.0f;
	phaserPosition[2] = 0.0f;

	radarLightAngle = 0.0f;
	phaserPlaying = AL_FALSE;


	/* initialize GLUT */
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowPosition(200, 100);
	glutInitWindowSize(320, 240);
	glutCreateWindow("XL Demo");
	
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);
	glutReshapeFunc(reshape);
	glutIdleFunc(idle);

	glEnable(GL_CULL_FACE);

	
	/* initialize ALUT */
	alutInit(&argc, argv);


   	/* set up the buffers */
   	alGenBuffers(1, &radarBuffer);
   	alGenBuffers(1, &phaserBuffer);
   	
   	alutLoadWAV(kRadarFileName, &format, &data, &size, &freq);
   	alBufferData(radarBuffer, format, data, size, freq);
	free(data);

   	alutLoadWAV(kPhaserFileName, &format, &data, &size, &freq);
   	alBufferData(phaserBuffer, format, data, size, freq);
	free(data);


   	/* set up the sources */
   	alGenSources(1, &radarSource);
   	alGenSources(1, &phaserSource);

	alSourcefv(radarSource, AL_POSITION, radarPosition);
	alSourcef(radarSource, AL_GAIN, 1.0f);
	alSourcef(radarSource, AL_PITCH, 1.0f);
	alSourcei(radarSource, AL_BUFFER, radarBuffer);
	alSourcei(radarSource, AL_LOOPING, AL_TRUE);
	
	alSourcefv(phaserSource, AL_POSITION, phaserPosition);
	alSourcef(phaserSource, AL_GAIN, 1.0f);
	alSourcef(phaserSource, AL_PITCH, 1.0f);
	alSourcei(phaserSource, AL_BUFFER, phaserBuffer);
	alSourcei(phaserSource, AL_LOOPING, AL_FALSE);

	/* start the radar */
	alSourcePlay(radarSource);


	/* GLUT event loop */
	glutMainLoop();
	
	
	/* shutdown alut */
	alutExit();

	return 0;
}
