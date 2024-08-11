#version 330

// Input attributes
in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D texture0;

// Output fragment color
out vec4 finalColor;

// Window size
uniform float size;

void main()
{
    vec4 texelColor = texture(texture0, fragTexCoord);

    vec2 uv = vec2(gl_FragCoord.x, gl_FragCoord.y) / size;
    vec2 centre = vec2(0.5, 0.5);
    float dist = 1 - distance(uv, centre);

    float falloff = 0.005;

    texelColor.a = smoothstep(0.5, 0.5 + falloff, dist);

    finalColor = texelColor;
}