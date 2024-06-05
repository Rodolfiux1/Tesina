import pandas as pd
import numpy as np
import seaborn as sns
from sklearn.ensemble import RandomForestClassifier
from sklearn.model_selection import train_test_split, RandomizedSearchCV
import matplotlib.pyplot as plt

datos = pd.read_csv('Base de datos1.csv')
datos['Fecha'] = pd.to_datetime(datos['Fecha'])
datos['Hora'] = datos['Fecha'].dt.hour
datos['Desv_Humedad_Suelo'] = datos.groupby('Hora')['Humedad Suelo'].transform(np.std)

plt.figure(figsize=(14, 8))
sns.boxplot(x='Hora', y='Desv_Humedad_Suelo', data=datos)
plt.title('Box Plot de la Desviación Estándar de la Humedad del Suelo por Hora')
plt.xlabel('Hora del día')
plt.ylabel('Desviación Estándar de la Humedad del Suelo')
plt.xticks(rotation=45)
plt.show()

datos['Necesita Riego'] = datos['Necesita Riego'].map({'No': 0, 'Si': 1})
datos['Es Optimo'] = datos['Es Optimo'].map({'No': 0, 'Si': 1})
umbral_desviacion = datos['Desv_Humedad_Suelo'].quantile(0.25)
datos['Es_Buena_Hora_Para_Regar'] = (datos['Desv_Humedad_Suelo'] <= umbral_desviacion).astype(int)

X = datos[['Hora', 'Temperatura', 'Humedad', 'Humedad Suelo', 'Desv_Humedad_Suelo', 'Necesita Riego', 'Es Optimo']]
y = datos['Es_Buena_Hora_Para_Regar']

X_train, X_temp, y_train, y_temp = train_test_split(X, y, test_size=0.3, random_state=42)

X_val, X_test, y_val, y_test = train_test_split(X_temp, y_temp, test_size=(1/3), random_state=42) 

parametros_rf = {
    'n_estimators': [100, 200, 300, 400, 500],
    'max_features': ['sqrt', 'log2'],
    'max_depth': [None, 10, 20, 30, 40, 50],
    'min_samples_split': [2, 5, 10],
    'min_samples_leaf': [1, 2, 4]
}

rf_model = RandomForestClassifier(random_state=42)
rf_random = RandomizedSearchCV(estimator=rf_model, param_distributions=parametros_rf, n_iter=100, cv=3, verbose=2, random_state=42, n_jobs=-1)
rf_random.fit(X_train, y_train)

print("Mejores parámetros:", rf_random.best_params_)

mejor_modelo = rf_random.best_estimator_
importancias = mejor_modelo.feature_importances_
nombres_caracteristicas = X.columns
df_importancias = pd.DataFrame({'caracteristica': nombres_caracteristicas, 'importancia': importancias})
df_importancias = df_importancias.sort_values(by='importancia', ascending=False)

plt.figure(figsize=(10, 6))
plt.barh(df_importancias['caracteristica'], df_importancias['importancia'], color='blue')
plt.xlabel('Importancia')
plt.title('Importancia de las Características en el Modelo de Random Forest')
plt.gca().invert_yaxis()
plt.show()

hora_predicha = mejor_modelo.predict(X_test)
hora_predicha_filtrada = X_test.loc[hora_predicha == 1, 'Hora']
plt.figure(figsize=(10, 6))
plt.boxplot(hora_predicha_filtrada, vert=False)
plt.xlabel('Hora del día')
plt.title('Distribución de las Horas Óptimas para Regar')
plt.yticks([])
plt.show()

hora_moda = hora_predicha_filtrada.mode()
if not hora_moda.empty:
    print('La hora más recomendada para regar es:', hora_moda[0])
else:
    print('No se pudo determinar una hora clara como la más recomendada. Revisar los datos.')
