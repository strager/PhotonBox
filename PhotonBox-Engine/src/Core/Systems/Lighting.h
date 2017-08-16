#ifndef LIGHTING_H
#define LIGHTING_H

class LightEmitter;
#include <map>
#include <vector>
#include <typeindex>

class Lighting {
public:
	template<class T>
	static void addLight(T* light) {
		
		if (_lights.find(typeid(light)) != _lights.end()) {
			_lights[typeid(light)].push_back(light);
		}
		else {
			std::vector<T*> vec;
			vec.push_back(light);
			_lights.insert(std::pair<std::type_index, std::vector<LightEmitter*>>(typeid(light), vec));
		}
	}


	static void removeLight(LightEmitter* light);

	template<class T>
	static std::vector<T*>& getLights() {
		if (_lights.find(typeid(T)) == _lights.end()) {
			std::cerr << "Lights not found!" << std::endl;
//			return _lights.end();
		}
		return (std::vector<T*>&)_lights[typeid(T)];
	}
private:
	static std::map<std::type_index, std::vector<LightEmitter*>> _lights;
	
};

#endif // LIGHTING_H
