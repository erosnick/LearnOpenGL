#version 400

in vec2 uv;

layout (location = 0) out vec4 fragColor;

uniform BlobSettings{
	vec4 InnerColor;
	vec4 OuterColor;
	float RadiusInner;
	float RadiusOuter;
} Blob;

void main() {
	float dx = uv.x - 0.5;
	float dy = uv.y - 0.5;

	float dist = sqrt(dx * dx + dy * dy);

	fragColor = mix(Blob.InnerColor, Blob.OuterColor, smoothstep(Blob.RadiusInner, Blob.RadiusOuter, dist));
}