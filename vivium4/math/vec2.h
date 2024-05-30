#pragma once

#include <cstdint>
#include <utility>
#include <string>
#include <format>
#include <cmath>

// TODO: .inl file

namespace Vivium {
	template <typename T = float>
	struct Vec2 {
		T x, y;

		Vec2() = default;
		explicit Vec2(T s) : x(s), y(s) {}
		Vec2(T x, T y) : x(x), y(y) {}

		Vec2 operator+(Vec2 v) const { return Vec2(x + v.x, y + v.y); }
		Vec2 operator-(Vec2 v) const { return Vec2(x - v.x, y - v.y); }
		Vec2 operator*(Vec2 v) const { return Vec2(x * v.x, y * v.y); }
		Vec2 operator/(Vec2 v) const { return Vec2(x / v.x, y / v.y); }

		friend Vec2 operator*(T s, Vec2 v) { return Vec2(s * v.x, s * v.y); }
		friend Vec2 operator/(T s, Vec2 v) { return Vec2(s / v.x, s / v.y); }
		friend Vec2 operator*(Vec2 v, T s) { return Vec2(v.x * s, v.y * s); }
		friend Vec2 operator/(Vec2 v, T s) { return Vec2(v.x / s, v.y / s); }

		void operator+=(Vec2 v) { x += v.x; y += v.y; }
		void operator-=(Vec2 v) { x -= v.x; y -= v.y; }
		void operator*=(Vec2 v) { x *= v.x; y *= v.y; }
		void operator/=(Vec2 v) { x /= v.x; y /= v.y; }

		void operator*=(T s) { x *= s; y *= s; }
		void operator/=(T s) { x /= s; y /= s; }

		Vec2 operator-() const { return Vec2(-x, -y); }

		bool operator<(Vec2 v) const { return x == v.x ? x < v.y : x < v.x; }

		bool operator==(Vec2 v) const { return x == v.x && y == v.y; }
		bool operator!=(Vec2 v) const { return x != v.x || y != v.y; }

		static Vec2 triple_product(Vec2 a, Vec2 b, Vec2 c) { return right(c) * cross(a,b); }
		static T orient(Vec2 a, Vec2 b, Vec2 c) { return cross(b - a, c - a); }

		static Vec2 floor(Vec2 v) { return Vec2(std::floor(v.x), std::floor(v.y)); }
		static Vec2 ceil(Vec2 v) { return Vec2(std::ceil(v.x), std::ceil(v.y)); }

		static Vec2 normalise(Vec2 v) { float l = length(v); return l == 0.0f ? Vec2(0) : v / length(v); }
		static Vec2 right(Vec2 v) { return Vec2(-v.y, v.x); }
		static Vec2 left(Vec2 v) { return Vec2(v.y, -v.x); }
		static T dot(Vec2 a, Vec2 b) { return a.x * b.x + a.y * b.y; }
		static T cross(Vec2 a, Vec2 b) { return a.x * b.y - a.y * b.x; }
		static T length(Vec2 v) { return std::sqrt(dot(v, v)); }

		template <typename U>
		operator Vec2<U>() const { return Vec2<U>((U)x, (U)y); }
	};

	// From boost combine_hash afaik
	template <typename T>
	struct vector2_hash {
		std::size_t operator()(const Vec2<T>& v) {
			std::hash<T> hasher;
			std::size_t hash = 0;
			hash ^= hasher(v.x) * 0x9e3779b9 + (hash << 6) + (hash >> 2);
			hash ^= hasher(v.y) * 0x9e3779b9 + (hash << 6) + (hash >> 2);

			return hash;
		}
	};
}

#define I32x2 Vivium::Vec2<int32_t>
#define I64x2 Vivium::Vec2<int64_t>
#define U32x2 Vivium::Vec2<uint32_t>
#define U64x2 Vivium::Vec2<uint64_t>
#define F32x2 Vivium::Vec2<float>
#define F64x2 Vivium::Vec2<double>