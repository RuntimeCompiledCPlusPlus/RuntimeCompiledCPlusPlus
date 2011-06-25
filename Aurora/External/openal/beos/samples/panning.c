/* xldemo.c - rewritten to use a right hand coordinate system */

#include <AL/al.h>
#include <AL/alut.h>
#include <AL/alc.h>
#include <GL/gl.h>
#include <GL/glut.h>

#include <stdio.h>
#include <time.h>

static void display( void );
static void keyboard( unsigned char key, int x, int y );
static void reshape( int w, int h );
static void init( void );

static ALuint left = 0;
static ALuint right = 0;


static void display( void )
{
    static time_t then = 0;
	static ALboolean toggle = AL_FALSE;
    time_t now;

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity( );

    /* Draw radiation cones. */
    glPushMatrix( );
    glRotatef( 90, 0.0, 1.0, 0.0 );
    glTranslatef( -2.0, 0.0, 4.0 );

    glutWireCone( 5.0 , 4.0, 20, 20);
    glPopMatrix( );

    glPushMatrix( );
    glRotatef( 90, 0.0, 1.0, 0.0 );
    glTranslatef( 2.0, 0.0, 4.0 );
    glutWireCone( 5.0 , 4.0, 20, 20);
    glPopMatrix( );

    /* In future, draw side to side moving wall for occlusion in AL. */

    glutWireCube( 20.0 );
    /* In future, define this also as the room for AL. */

    now = time( NULL );

    /* Switch between left and right boom every two seconds. */
    if ( now - then > 2 ) {
		then = now;
	
		if( toggle == AL_TRUE ) {
		    alSourcePlay( left );
		    toggle = AL_FALSE;
		}
		else {
		    alSourcePlay( right );
		    toggle = AL_TRUE;
		}
    }

    glutSwapBuffers( );
    glutPostRedisplay( );
}

static void keyboard( unsigned char key, int x, int y )
{
    switch ( key ) {
    case 27:
		exit( 0 );
		break;
    default:
		break;
    }

}

static void reshape( int w, int h )
{
    glViewport( 0, 0, w, h );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity( );
    gluPerspective( 60.0f, (float) w / (float) h, 0.1f, 1024.0f );
}

static void init( void )
{
    ALfloat zeroes[] = { 0.0f, 0.0f, 0.0f };
    ALfloat front[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };
    ALfloat back[] = { 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f };
    ALuint sources[2];
    ALuint boom;
    void* wave;
    ALsizei size;
    ALsizei freq;
    ALenum format;

    glClearColor( 0.0, 0.0, 0.0, 0.0 );
    glShadeModel( GL_SMOOTH );

    alListenerfv( AL_POSITION, zeroes );
    alListenerfv( AL_VELOCITY, zeroes );
    alListenerfv( AL_ORIENTATION, front );
 
    if( alGenBuffers( 1, &boom ) != 1 ) {
		fprintf( stderr, "aldemo: couldn't generate samples\n" );
		exit( 1 );
    }

    alutLoadWAV( "data/boom.wav", &format, &wave, &size, &freq );
    alBufferData( boom, format, wave, size, freq );

    if( alGenSources( 2, sources ) != 2 ) {
		fprintf( stderr, "aldemo: couldn't generate sources\n" );
		exit( 1 );
    }

    left = sources[0];
    right = sources[1];

    alSource3f( left, AL_POSITION, 2.0, 0.0, 4.0 );
    alSourcefv( left, AL_VELOCITY, zeroes );
    alSourcefv( left, AL_ORIENTATION, back );
    /*alSourcei( left, AL_CONE, 60 );*/
    alSourcei( left, AL_BUFFER, boom );

    alSource3f( right, AL_POSITION, -2.0, 0.0, 4.0 );
    alSourcefv( right, AL_VELOCITY, zeroes );
    alSourcefv( right, AL_ORIENTATION, back );
    /*alSourcei( right, AL_CONE, 60 );*/
    alSourcei( right, AL_BUFFER, boom );

    /* PENDING Create a material, define it for the room. */
}

int main( int argc, char* argv[] )
{
    /* Initialize GLUT. */
    glutInit( &argc, argv );

    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
    glutInitWindowSize( 320, 240 );
    glutInitWindowPosition( 100, 100 );
    glutCreateWindow( argv[0] );

    glutReshapeFunc( reshape ); 
    glutDisplayFunc( display );
    glutKeyboardFunc( keyboard );

    /* Initialize ALUT. */
    alutInit( &argc, argv );

    init( );

    glutMainLoop( );

	alutExit();

    return 0;
}
