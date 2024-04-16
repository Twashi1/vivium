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
	private:
		using is_vector_type = std::true_type;

	public:
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

		Vec2 floor() const { return Vec2(std::floor(x), std::floor(y)); }
		Vec2 ceil() const { return Vec2(std::ceil(x), std::ceil(y)); }

		T dot(Vec2 v) const { return x * v.x + y * v.y; }
		T cross(Vec2 v) const { return x * v.y - y * v.x; }
		T length() const { return std::sqrt(x * x + y * y); }

		Vec2 normalise() const { return *this / std::sqrt(x * x + y * y); }

		Vec2 left() const { return Vec2(y, -x); }
		Vec2 right() const { return Vec2(-y, x); }

		static Vec2 triple_product(Vec2 a, Vec2 b, Vec2 c) { return c.right() * a.cross(b); }
		static T orient(Vec2 a, Vec2 b, Vec2 c) { return (b - a).cross(c - a); }

		static Vec2 normalise(Vec2 v) { return v.normalise(); }
		static Vec2 right(Vec2 v) { return v.right(); }
		static Vec2 left(Vec2 v) { return v.left(); }
		static T dot(Vec2 a, Vec2 b) { return a.dot(b); }
		static T cross(Vec2 a, Vec2 b) { return a.cross(b); }
		static T length(Vec2 v) { return v.length(); }

		template <typename U>
		operator Vec2<U>() const { return Vec2<U>((U)x, (U)y); }
	};

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

	template <typename T>
	concept c_vec2 = requires (T a) {
		a.x;
		a.y;
	} && T::is_vector_type.value();
}

#define I32x2 Vivium::Vec2<int32_t>
#define I64x2 Vivium::Vec2<int64_t>
#define U32x2 Vivium::Vec2<uint32_t>
#define U64x2 Vivium::Vec2<uint64_t>
#define F32x2 Vivium::Vec2<float>
#define F64x2 Vivium::Vec2<double>

namespace std {
	template <Vivium::c_vec2 T>
	struct formatter<T> : formatter<string> {
		auto format(T v, format_context& ctx) {
			return formatter<string>::format(
				std::format("[{}, {}]", v.x, v.y),
				ctx
			);
		}
	};
}