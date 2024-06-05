import pandas as pd
import pytz
import numpy as np

df_temp_tierra = pd.read_csv('Untitled-TemperaturaDeTierra.csv')
df_humedad = pd.read_csv('Untitled-Humedad.csv')
df_humedad_suelo = pd.read_csv('Untitled-humedad_Del_suelo.csv')
df_temperatura = pd.read_csv('Untitled-Temperatura.csv')

df_temp_tierra['Fecha'] = pd.to_datetime(df_temp_tierra['time'], utc=True)
df_humedad['Fecha'] = pd.to_datetime(df_humedad['time'], utc=True)
df_humedad_suelo['Fecha'] = pd.to_datetime(df_humedad_suelo['time'], utc=True)
df_temperatura['Fecha'] = pd.to_datetime(df_temperatura['time'], utc=True)

df_temp_tierra['Temperatura Tierra'] = df_temp_tierra['value'].round(2)
df_humedad['Humedad'] = df_humedad['value'].round(2)
df_humedad_suelo['Humedad Suelo'] = df_humedad_suelo['value'].round(2)
df_temperatura['Temperatura'] = df_temperatura['value'].round(2)

for df in [df_temp_tierra, df_humedad, df_humedad_suelo, df_temperatura]:
    df.drop(columns=['time', 'value'], inplace=True)

df_final = df_temp_tierra
for df in [df_humedad, df_humedad_suelo, df_temperatura]:
    df_final = pd.merge(df_final, df, on='Fecha', how='outer')

start_date = pd.to_datetime('2024-05-07T04:29:46.015247468Z', utc=True)
df_final = df_final[df_final['Fecha'] >= start_date].sort_values(by='Fecha')

df_final['Necesita Riego'] = np.where(
    (df_final['Temperatura'] > 25) & (df_final['Humedad Suelo'] < 60) | (df_final['Humedad Suelo'] < 45), 
    'Si', 
    'No'
)

df_final['Es Optimo'] = np.where(
    df_final['Temperatura'] < 25, 
    'Si', 
    'No'
)

def select_samples(group):
    step = 80 if len(group) % 2 == 0 else 81  
    return group.iloc[::step]

sampled_df = select_samples(df_final)

timezone_mexico = pytz.timezone('America/Mexico_City')
sampled_df['Fecha'] = sampled_df['Fecha'].dt.tz_convert(timezone_mexico)

sampled_df['Fecha'] = sampled_df['Fecha'].dt.strftime('%d/%m/%Y %H:%M:%S')

sampled_df.to_csv('Base de datos1.csv', index=False)


